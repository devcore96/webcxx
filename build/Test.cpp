#include "Test.h"
#include <cstdio>
#include <array>
#include <sstream>
#include <iostream>

Test::Test(const std::string& outputName, const std::string& compiler)
    : Target(outputName, compiler), result(std::make_shared<TestResult>())
{ }

bool Test::needsRecompilation()
{
    return true;
}

std::shared_ptr<BuildResult> Test::build()
{
    // First, call parent class's build to handle linking   
    auto buildResult = Target::build();

    if (buildResult) {
        // Copy buildResult data to result
        result->success  = buildResult->success;
        result->warnings = buildResult->warnings;
        result->errors   = buildResult->errors;
        result->output   = buildResult->output;

        if (!buildResult->success)
        {
            // Build failed, return result
            result->testSuccess = false;
            return result;
        }
    }

    // Build succeeded, now run the test program
    std::string cmdStr = "./" + outputName;

    std::array<char, 128> buffer;
    std::string output;

    // Open a pipe to read the output
    FILE* pipe = popen((cmdStr + " 2>&1").c_str(), "r");
    if (!pipe)
    {
        result->testSuccess = false;
        result->errors.push_back("Failed to execute test command.");
        return result;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        output += buffer.data();
    }

    int returnCode = pclose(pipe);

    result->testOutput = output;

    if (returnCode == 0)
    {
        result->testSuccess = true;
    }
    else
    {
        result->testSuccess = false;
    }

    return result;
}

std::shared_ptr<TestResult> Test::getResult() const {
    return result;
}
