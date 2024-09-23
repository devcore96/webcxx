#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <regex>
#include <fstream>
#include <format>

#include "Terminal.h"
#include "ProgressBar.h"
#include "Project.h"
#include "BuildManager.h"
#include "Test.h"

namespace fs = std::filesystem;

using cu     = std::shared_ptr<CompileUnit>;
using target = std::shared_ptr<Target>;
using test   = std::shared_ptr<Test>;

std::map<std::string, cu> cus;

cu& getCU(std::string name) {
    // Normalize the path
    fs::path path = fs::path(name).lexically_normal();
    name = path.string();

    if (!cus.contains(name))
        cus.insert(std::make_pair(name, std::make_shared<CompileUnit>(name, std::format(".out/{}.o", name))));

    return cus[name];
}

// Function that finds all .cpp files, with an option to skip "jobs" and "tests" directories
std::vector<fs::path> find_cpp_files(const fs::path& folder, bool skipJobsAndTests = true) {
    std::vector<fs::path> cpp_files;

    // Use a recursive directory iterator to traverse the folder
    for (const auto& entry : fs::recursive_directory_iterator(folder)) {
        // Check if we need to skip "jobs" or "tests" directories
        if (skipJobsAndTests) {
            std::string path_str = entry.path().string();
            if (path_str.find("/jobs/") != std::string::npos || path_str.find("/tests/") != std::string::npos) {
                continue;
            }
        }

        // If it's a regular file and has a .cpp extension, add it to the result
        if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
            cpp_files.push_back(entry.path());
        }
    }

    return cpp_files;
}

// Structure to hold macro information
struct MacroInfo {
    std::vector<std::string> flags;
    std::vector<std::string> sources;
    std::vector<std::string> libraries;
    std::vector<std::string> libraryPaths;
    std::vector<std::string> includePaths;
};

// Function to parse macros from a source file
MacroInfo parseMacros(const fs::path& sourceFile) {
    MacroInfo info;
    std::ifstream file(sourceFile);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << sourceFile << std::endl;
        return info;
    }

    std::string line;
    std::regex macroRegex(R"(\b(FLAG|SOURCE|LIBRARY|LIBRARY_PATH|INCLUDE_PATH)\s*\(\s*\"([^\"]+)\"\s*\))");

    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, macroRegex)) {
            std::string macroName = match[1];
            std::string value = match[2];

            if (macroName == "FLAG") {
                info.flags.push_back(value);
            } else if (macroName == "SOURCE") {
                info.sources.push_back(value);
            } else if (macroName == "LIBRARY") {
                info.libraries.push_back(value);
            } else if (macroName == "LIBRARY_PATH") {
                info.libraryPaths.push_back(value);
            } else if (macroName == "INCLUDE_PATH") {
                info.includePaths.push_back(value);
            }
        }
    }

    file.close();
    return info;
}

int main()
{
    // Set compiler flags
    std::vector<std::string> flags;

    flags.push_back("-std=c++26");
    flags.push_back("-Ofast");
    flags.push_back("-ffast-math");
    flags.push_back("-pthread");
    flags.push_back("-w");

    // Set include paths
    std::vector<std::string> includePaths;

    includePaths.push_back(".");

    // Set library paths
    std::vector<std::string> libraryPaths;

    // Create the project
    Project project;

    // index.cgi - main app
    target index = std::make_shared<Target>("index.cgi");

    // Add index.cgi to project
    project.targets.push_back(index);

    // Add jobs to project
    // Find all .cpp files in "app/jobs" directory
    auto job_sources = find_cpp_files("app/jobs", false);

    // Create a map to store job targets
    std::map<std::string, target> jobs;

    for (const auto& source : job_sources) {
        // Get the relative path without "app/" and without ".cpp"
        std::string relative_path = fs::relative(source, "app").string();
        if (relative_path.ends_with(".cpp")) {
            relative_path = relative_path.substr(0, relative_path.size() - 4); // Remove ".cpp"
        }

        // Create a Target for each job
        target job_target = std::make_shared<Target>(relative_path);

        // Add the main source file as a compile unit
        job_target->addCompileUnit(getCU(source.string()));

        // Store the job target
        jobs[relative_path] = job_target;

        // Add the job target to the project
        project.targets.push_back(job_target);
    }

    // Add tests to project
    // Find all .cpp files in "app/tests" directory
    auto test_sources = find_cpp_files("app/tests", false);

    // Create a map to store test targets
    std::map<std::string, test> tests;

    for (const auto& source : test_sources) {
        // Get the relative path without "app/" and without ".cpp"
        std::string relative_path = fs::relative(source, "app").string();
        if (relative_path.ends_with(".cpp")) {
            relative_path = relative_path.substr(0, relative_path.size() - 4); // Remove ".cpp"
        }

        // Create a Test for each test
        test test_target = std::make_shared<Test>(relative_path);

        // Add the main source file as a compile unit
        test_target->addCompileUnit(getCU(source.string()));

        // Store the test target
        tests[relative_path] = test_target;

        // Add the test target to the project
        project.targets.push_back(test_target);
    }

    // Add compile units to index.cgi
    auto app_sources    = find_cpp_files("app/", true);
    auto routes_sources = find_cpp_files("routes/", true);
    auto pages_sources  = find_cpp_files("pages/", true);

    for (auto& source : app_sources) index->addCompileUnit(getCU(source.string()));
    for (auto& source : routes_sources) index->addCompileUnit(getCU(source.string()));
    for (auto& source : pages_sources) index->addCompileUnit(getCU(source.string()));

    // Add source files to jobs
    for (auto& [jobName, jobTarget] : jobs) {
        // Get the main source file for this job
        auto mainSource = jobTarget->getCompileUnits().front()->getSourcePath();

        // Parse macros from the main source file
        MacroInfo macros = parseMacros(mainSource);

        // Add additional source files
        for (const auto& sourcePath : macros.sources) {
            jobTarget->addCompileUnit(getCU(sourcePath));
        }

        // Add flags, libraries, include paths, and library paths
        jobTarget->addCompileFlags(macros.flags);
        jobTarget->addLibraries(macros.libraries);
        jobTarget->addLibraryPaths(macros.libraryPaths);
        jobTarget->addIncludePaths(macros.includePaths);

        // Add flags and include paths to compile units
        const auto& units = jobTarget->getCompileUnits();
        for (const auto& unit : units) {
            unit->addCompileFlags(macros.flags);
            unit->addIncludePaths(macros.includePaths);
        }
    }

    // Add source files to tests
    for (auto& [testName, testTarget] : tests) {
        // Get the main source file for this test
        auto mainSource = testTarget->getCompileUnits().front()->getSourcePath();

        // Parse macros from the main source file
        MacroInfo macros = parseMacros(mainSource);

        // Add additional source files
        for (const auto& sourcePath : macros.sources) {
            testTarget->addCompileUnit(getCU(sourcePath));
        }

        // All tests depend on build/Terminal.cpp for colored output
        testTarget->addCompileUnit(getCU("build/Terminal.cpp"));

        // Add flags, libraries, include paths, and library paths
        testTarget->addCompileFlags(macros.flags);
        testTarget->addLibraries(macros.libraries);
        testTarget->addLibraryPaths(macros.libraryPaths);
        testTarget->addIncludePaths(macros.includePaths);

        // Add flags and include paths to compile units
        const auto& units = testTarget->getCompileUnits();
        for (const auto& unit : units) {
            unit->addCompileFlags(macros.flags);
            unit->addIncludePaths(macros.includePaths);
        }
    }

    // Add flags and library paths to all targets
    for (auto& target : project.targets) {
        for (auto& flag : flags) {
            target->addCompileFlag(flag);
        }

        for (auto& path : libraryPaths) {
            target->addLibraryPath(path);
        }
    }

    // Add flags and include paths to all compile units
    for (auto& unit : cus) {
        for (auto& flag : flags) {
            unit.second->addCompileFlag(flag);
        }

        for (auto& path : includePaths) {
            unit.second->addIncludePath(path);
        }
    }

    // Add libraries to index.cgi
    index->addLibrary("cgicc");
    index->addLibrary("png");
    index->addLibrary("curl");
    index->addLibrary("mysqlcppconn");
    index->addLibrary("mysqlcppconn8");
    index->addLibrary("stdc++exp");

    // Create build manager
    BuildManager manager(project);

    manager.build();

    return 0;
}
