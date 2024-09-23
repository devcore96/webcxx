#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include "ICompilable.h"
#include "Project.h"
#include "ProgressBar.h"

class BuildManager
{
public:
    BuildManager(Project& project);

    void build();

private:
    Project& project;

    // Threads
    unsigned int numThreads;
    std::vector<std::thread> threadPool;

    // Synchronization
    std::mutex mtx;

    // Build queues and status
    std::vector<std::shared_ptr<ICompilable>> toBuild;
    std::vector<std::shared_ptr<ICompilable>> currentlyBuilding;
    std::vector<std::shared_ptr<ICompilable>> built;

    // Counters and statuses
    int totalSteps;
    int currentStep;
    int warningsCount;
    int errorsCount;

    int testsSuccessful;
    int testsFailed;
    int testsPending;

    std::string lastCompletedStepText;

    std::unique_ptr<ProgressBar> progressBar;

    // Other methods
    void workerThread();
    void updateProgress();
    void displayProgressBar();

    // Helper methods
    void collectCompilables();
    void initializeBuildStatus();
};
