#pragma once

#include "BuildResult.h"

class TestResult : public BuildResult
{
public:
    bool testSuccess = false; // Whether the test was successful
    std::string testOutput;   // The raw text output of the test
};
