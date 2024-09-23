#pragma once

#include <memory>
#include "BuildResult.h"

class ICompilable
{
public:
    virtual ~ICompilable() = default;

    // Build the compilable unit
    virtual std::shared_ptr<BuildResult> build() = 0;

    // Check if the compilable unit needs recompilation
    virtual bool needsRecompilation() = 0;

    // Check if the compilable unit can be built
    virtual bool canBuild() = 0;

    // Get the name of the compilable unit (e.g., output name)
    virtual std::string getName() const = 0;
};
