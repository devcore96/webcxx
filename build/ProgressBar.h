#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include "Terminal.h"

class ProgressBar
{
public:
    ProgressBar(int totalSteps);
    ~ProgressBar(); // Destructor to restore cursor if needed
    void update(int currentStep, const std::string& stepDescription,
                const std::string& testStatus,
                const std::string& warnings);
    void finish();

private:
    int totalSteps;
    int barWidth;
    bool useUnicodeBlocks;
    int numInfoLines; // Number of information lines under the progress bar
    bool cursorHidden; // To track cursor visibility

    int getConsoleWidth();
    int numDigits(int number);
    bool detectUnicodeSupport();
};
