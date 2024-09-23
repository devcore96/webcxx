#include "BuildManager.h"
#include <thread>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <condition_variable>
#include <set>
#include <format>
#include "Test.h"

BuildManager::BuildManager(Project& proj)
    : project(proj)
{
    // 1. Check how many threads are available in the host system
    numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) {
        numThreads = 4; // Fallback to 4 if hardware_concurrency returns 0
    }

    // 2. Collect all ICompilables
    collectCompilables();

    // Initialize build status
    initializeBuildStatus();

    lastCompletedStepText = "Building...";

    progressBar = std::make_unique<ProgressBar>(toBuild.size());
}

void BuildManager::collectCompilables()
{
    // Collect all compile units
    std::set<std::shared_ptr<CompileUnit>> compileUnitSet;

    for (const auto& target : project.targets) {
        const auto& compileUnits = target->getCompileUnits();
        for (const auto& cu : compileUnits) {
            compileUnitSet.insert(cu);
        }
    }

    // Convert set to vector
    std::vector<std::shared_ptr<CompileUnit>> compileUnits(compileUnitSet.begin(), compileUnitSet.end());

    // Collect all ICompilables
    toBuild.clear();
    built.clear();
    currentlyBuilding.clear();

    // Add all CompileUnits
    for (const auto& cu : compileUnits) {
        toBuild.push_back(cu);
    }

    // Add all Targets
    for (const auto& target : project.targets) {
        toBuild.push_back(target);
    }
}

void BuildManager::initializeBuildStatus()
{
    totalSteps = 0;
    currentStep = 0;

    warningsCount = 0;
    errorsCount = 0;

    testsSuccessful = 0;
    testsFailed = 0;
    testsPending = 0;

    lastCompletedStepText = "";

    // Calculate totalSteps
    int numCompileUnits = 0;
    int numTargets = 0;

    for (const auto& compilable : toBuild) {
        if (std::dynamic_pointer_cast<CompileUnit>(compilable)) {
            numCompileUnits++;
        } else if (auto target = std::dynamic_pointer_cast<Target>(compilable)) {
            numTargets++;
            if (std::dynamic_pointer_cast<Test>(compilable)) {
                testsPending++;
            }

            if (!target->needsRecompilation()) {
                currentStep++;
            }
        }
    }

    totalSteps = numCompileUnits + numTargets;
}

void BuildManager::build()
{
    // Start worker threads
    for (unsigned int i = 0; i < numThreads; ++i) {
        threadPool.emplace_back(&BuildManager::workerThread, this);
    }

    // In the main thread, we can monitor progress and update the progress bar
    while (currentStep < totalSteps) {
        displayProgressBar();

        // Sleep for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // Wait for all threads to finish
    for (auto& t : threadPool) {
        t.join();
    }

    // Final progress update
    displayProgressBar();

    std::cout << "\n\n\n\nBuild successful!\n\n"
                 "Tests report:";

    for (auto& item : built) {
        if (auto test = std::dynamic_pointer_cast<Test>(item)) {
            auto result = test->getResult();

            std::cout << '\n';

            if (result->success) {
                std::cout << Terminal::Color::Green << Terminal::Color::Default << test->getName() << "\n";
                std::cout << result->testOutput;
            } else {
                std::cout << Terminal::Color::Red << "[Could not build] " << Terminal::Color::Default << test->getName() << "\n";
                std::cout << result->output << "\n";
            }
        }
    }

    std::cout << std::flush;
}

void BuildManager::workerThread()
{
    while (currentStep < totalSteps) {
        std::shared_ptr<ICompilable> itemToBuild = nullptr;

        // a. lock the mutex
        {
            std::unique_lock<std::mutex> lock(mtx);

            // b. If no ICompilables are left in the list of things to compile then release the mutex and end this thread
            if (toBuild.empty()) {
                return;
            }

            // c. check if any target/test that needs to be built can be built and is not already being built. if any match this query, select that target.
            for (auto it = toBuild.begin(); it != toBuild.end(); ++it) {
                if (std::dynamic_pointer_cast<Target>(*it)) {
                    auto target = std::dynamic_pointer_cast<Target>(*it);
                    if (target->needsRecompilation() && target->canBuild()) {
                        // Check if it's already being built
                        if (std::find(currentlyBuilding.begin(), currentlyBuilding.end(), *it) == currentlyBuilding.end()) {
                            // Select this target
                            itemToBuild = *it;
                            // e. move it from the list of items to be built to the list of items being currently compiled
                            currentlyBuilding.push_back(itemToBuild);
                            toBuild.erase(it);
                            break;
                        }
                    }
                }
            }

            // d. if no targets/test can be built or all the ones that can be built are already being built then select the next CompileUnit which is not already being built by another thread and needs to be rebuilt.
            if (!itemToBuild) {
                for (auto it = toBuild.begin(); it != toBuild.end(); ++it) {
                    if (std::dynamic_pointer_cast<CompileUnit>(*it)) {
                        auto cu = std::dynamic_pointer_cast<CompileUnit>(*it);
                        if (cu->needsRecompilation()) {
                            // Check if it's already being built
                            if (std::find(currentlyBuilding.begin(), currentlyBuilding.end(), *it) == currentlyBuilding.end()) {
                                // Select this CompileUnit
                                itemToBuild = *it;
                                // e. move it from the list of items to be built to the list of items being currently compiled
                                currentlyBuilding.push_back(itemToBuild);
                                toBuild.erase(it);
                                break;
                            }
                        } else {
                            // If the CompileUnit doesn't need recompilation, we can consider it as built
                            built.push_back(*it);
                            toBuild.erase(it);
                            currentStep++;
                            break;
                        }
                    }
                }
            }

            // f. release the lock
        }

        // g. if nothing was selected then everything that needs to be compiled is waiting on something else to be compiled first. Wait for a bit, then repeat the loop.
        if (!itemToBuild) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // h. if something was selected, build it.
        try {
            auto buildResult = itemToBuild->build();

            // i. once it's built lock the mutex
            {
                std::unique_lock<std::mutex> lock(mtx);

                // j. move the item from the list of items being compiled to the list of compiled items
                auto it = std::find(currentlyBuilding.begin(), currentlyBuilding.end(), itemToBuild);
                if (it != currentlyBuilding.end()) {
                    currentlyBuilding.erase(it);
                }
                built.push_back(itemToBuild);

                // Update currentStep
                currentStep++;

                // k. update the warnings and error counters. also, update the test statuses if this was a test
                warningsCount += buildResult->warnings.size();
                errorsCount += buildResult->errors.size();

                if (auto test = std::dynamic_pointer_cast<Test>(itemToBuild)) {
                    testsPending--;
                    auto testResult = std::dynamic_pointer_cast<TestResult>(buildResult);
                    if (testResult && testResult->testSuccess) {
                        testsSuccessful++;
                        lastCompletedStepText = "Test " + itemToBuild->getName() + " OK";
                    } else {
                        testsFailed++;

                        lastCompletedStepText = "";
                        lastCompletedStepText = lastCompletedStepText + Terminal::Color::Red;
                        lastCompletedStepText += "Test " + itemToBuild->getName() + " failed";
                        lastCompletedStepText = lastCompletedStepText + Terminal::Color::Default;
                    }
                } else {
                    lastCompletedStepText = "Built " + itemToBuild->getName();

                    if (!buildResult->errors.empty()) {
                        std::cerr << "\n\n\n\n" << itemToBuild->getName() << ": build failed!\n\n"
                                << buildResult->output
                                << std::flush; 

                        exit(EXIT_FAILURE);
                    }
                }
            }
        } catch (const std::exception& e) {
            // Handle build exceptions
            std::unique_lock<std::mutex> lock(mtx);
            auto it = std::find(currentlyBuilding.begin(), currentlyBuilding.end(), itemToBuild);
            if (it != currentlyBuilding.end()) {
                currentlyBuilding.erase(it);
            }
            built.push_back(itemToBuild);
            currentStep++;
            errorsCount++;
            lastCompletedStepText = "Failed to build " + itemToBuild->getName() + ": " + e.what();
        }
    }
}

void BuildManager::displayProgressBar()
{
    std::string defaultColor, redColor, yellowColor, greenColor, grayColor;
    defaultColor = defaultColor + Terminal::Color::Default;
    redColor     = redColor     + Terminal::Color::Red;
    yellowColor  = yellowColor  + Terminal::Color::Yellow;
    greenColor   = greenColor   + Terminal::Color::Green;
    grayColor    = grayColor    + Terminal::Color::LightGray;

    progressBar->update(currentStep,
                        lastCompletedStepText,
                        std::format("Tests status: {}{} successful{}, {}{} failed{}, {}{} pending",
                                    greenColor, testsSuccessful, defaultColor,
                                    testsFailed > 0 ? redColor : greenColor, testsFailed, defaultColor,
                                    grayColor, testsPending),
                        std::format("{}{} warnings, {} errors",
                                    errorsCount > 0 ? redColor : (warningsCount > 0 ? yellowColor : greenColor),
                                    warningsCount,
                                    errorsCount)
                        );
    
    // Flush output
    std::cout.flush();
}
