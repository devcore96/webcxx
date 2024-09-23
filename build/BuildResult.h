#pragma once

#include <string>
#include <vector>

class BuildResult
{
public:
    virtual ~BuildResult() = default;  // Virtual destructor for polymorphism

    bool success = true;               // Whether the build was successful
    std::vector<std::string> warnings; // Any warnings that occurred in the build
    std::vector<std::string> errors;   // Any errors that occurred in the build
    std::string output;                // The raw text output of the build
};
