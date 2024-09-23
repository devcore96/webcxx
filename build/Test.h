#pragma once

#include "Target.h"
#include "TestResult.h"

class Test : public Target
{
private:
    std::shared_ptr<TestResult> result;

public:
    Test(const std::string& outputName, const std::string& compiler = "g++");

    virtual bool needsRecompilation() override;
    virtual std::shared_ptr<BuildResult> build() override;

    std::shared_ptr<TestResult> getResult() const;
};
