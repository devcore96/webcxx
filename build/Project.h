#pragma once

#include <vector>
#include <memory>
#include "Target.h"
#include "BuildResult.h"

struct Project {
    std::vector<std::shared_ptr<Target>>      targets; // This will contain both targets and tests
    std::vector<std::shared_ptr<BuildResult>> results; // This will contain the output of all targets and tests
};
