#include "ProgressBar.h"
#include <iomanip>   // For std::setw and std::setfill
#include <sstream>
#include <locale.h>  // For locale-related functions
#include <string.h>  // For strstr function

ProgressBar::ProgressBar(int totalSteps)
    : totalSteps(totalSteps), numInfoLines(3), cursorHidden(false) // Initialize cursorHidden
{
    // Detect Unicode support
    useUnicodeBlocks = detectUnicodeSupport();

    // Calculate the space needed for [step/total]
    int totalDigits    = numDigits(totalSteps);
    int stepTotalWidth = 3 + totalDigits * 2; // '[' + step + '/' + total + ']'

    // Calculate the space needed for percentage (always 5 characters)
    int percentageWidth = 5; // ' ' + percentage (3 chars) + '%'

    // Calculate the bar width
    barWidth = getConsoleWidth() - (stepTotalWidth + percentageWidth + 2); // Additional spaces

    if (barWidth < 10)
        barWidth = 10; // Minimum bar width

    // Hide the cursor
    std::cout << "\033[?25l"; // ANSI escape code to hide the cursor
    cursorHidden = true;

    // Save the cursor position to return to the progress bar
    std::cout << Terminal::Cursor::SavePosition;

    // Print initial empty lines for warnings and test status
    for (int i = 0; i < numInfoLines - 1; ++i)
    {
        std::cout << std::endl;
    }

    // Move the cursor to the position where the progress bar will be drawn
    std::cout << std::endl;

    // Print initial empty line for current step description
    std::cout << std::endl;

    // Move the cursor back up to the saved position
    for (int i = 0; i < numInfoLines; ++i)
    {
        std::cout << Terminal::Cursor::moveUp();
    }
}

// Destructor to restore the cursor if needed
ProgressBar::~ProgressBar()
{
    if (cursorHidden)
    {
        // Show the cursor
        std::cout << "\033[?25h"; // ANSI escape code to show the cursor
        cursorHidden = false;
    }
}

void ProgressBar::update(int currentStep, const std::string& stepDescription,
                         const std::string& testStatus,
                         const std::string& warnings)
{
    // Calculate progress
    double progress     = static_cast<double>(currentStep) / totalSteps;
    double filledUnits  = progress * barWidth;

    // Build progress bar string
    std::ostringstream barStream;

    // Declare totalDigits here
    int totalDigits = numDigits(totalSteps);

    // Prepare the left indicator [step/total] in white
    barStream << Terminal::Color::White;
    barStream << "[";
    barStream << std::setw(totalDigits) << std::setfill(' ') << currentStep;
    barStream << "/";
    barStream << totalSteps << "] ";

    // Build the progress bar without edges
    if (useUnicodeBlocks)
    {
        // Use detailed Unicode block characters
        int    fullBlocks     = static_cast<int>(filledUnits); // Number of full blocks
        double fractionalPart = filledUnits - fullBlocks;

        for (int i = 0; i < barWidth; ++i)
        {
            if (i < fullBlocks)
            {
                barStream << "█"; // Full block
            }
            else if (i == fullBlocks)
            {
                // Use fractional block
                if      (fractionalPart >= 7.0 / 8.0) barStream << "▉"; // Left seven eighths block
                else if (fractionalPart >= 6.0 / 8.0) barStream << "▊"; // Left three quarters block
                else if (fractionalPart >= 5.0 / 8.0) barStream << "▋"; // Left five eighths block
                else if (fractionalPart >= 4.0 / 8.0) barStream << "▌"; // Left half block
                else if (fractionalPart >= 3.0 / 8.0) barStream << "▍"; // Left three eighths block
                else if (fractionalPart >= 2.0 / 8.0) barStream << "▎"; // Left one quarter block
                else if (fractionalPart >= 1.0 / 8.0) barStream << "▏"; // Left one eighth block
                else                                  barStream << " "; // Empty part
            }
            else
            {
                barStream << " "; // Empty part
            }
        }
    }
    else
    {
        // Use simple ASCII block character
        int filledLength = static_cast<int>(filledUnits + 0.5); // Round to nearest integer

        for (int i = 0; i < barWidth; ++i)
        {
            if (i < filledLength)
                barStream << static_cast<char>(219); // Filled part
            else
                barStream << " "; // Empty part
        }
    }

    // Right indicator: percentage (always 5 characters)
    int percentage = static_cast<int>(progress * 100);

    // Prepare percentage string with padding
    barStream << " "
              << std::setw(3) << std::setfill(' ') << percentage << "%";
    barStream << Terminal::Color::Default; // Reset color

    // Move cursor to saved position (top of progress display)
    std::cout << Terminal::Cursor::RestorePosition;

    // Warnings/Errors with color (line 1)
    std::cout << Terminal::Cursor::ClearLine;
    std::cout << warnings << Terminal::Color::Default << std::flush;

    // Move down to test status line
    std::cout << Terminal::Cursor::moveDown();

    // Test Status with color (line 2)
    std::cout << Terminal::Cursor::ClearLine;
    std::cout << testStatus << Terminal::Color::Default << std::flush;

    // Move down to progress bar line
    std::cout << Terminal::Cursor::moveDown();

    // Progress bar line (line 3)
    std::cout << Terminal::Cursor::ClearLine;
    std::cout << barStream.str() << std::flush;

    // Move down to current step description line
    std::cout << Terminal::Cursor::moveDown();

    // Step Description (white) (line 4)
    std::cout << Terminal::Cursor::ClearLine;
    std::cout << Terminal::Color::White << stepDescription << Terminal::Color::Default << std::flush;

    // Move back up to the saved position
    std::cout << Terminal::Cursor::moveUp(3);
}

void ProgressBar::finish()
{
    // Do not clear the progress bar and info lines
    // Move cursor to the next line after the progress bar and info lines
    for (int i = 0; i < numInfoLines; ++i)
    {
        std::cout << Terminal::Cursor::moveDown();
    }
    std::cout << Terminal::Cursor::moveDown() << Terminal::Cursor::ClearLine;

    // Show the cursor
    if (cursorHidden)
    {
        std::cout << "\033[?25h"; // ANSI escape code to show the cursor
        cursorHidden = false;
    }
}

int ProgressBar::getConsoleWidth()
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
    {
        // Default width if retrieval fails
        return 80;
    }
    return w.ws_col;
}

int ProgressBar::numDigits(int number)
{
    int digits = 0;
    if (number <= 0)
        digits = 1; // Handle zero and negative numbers
    else
    {
        while (number)
        {
            number /= 10;
            digits++;
        }
    }
    return digits;
}

bool ProgressBar::detectUnicodeSupport()
{
    // Attempt to set the locale to the user's preferred locale
    char* locale = setlocale(LC_CTYPE, "");
    if (locale == nullptr)
    {
        // Unable to set locale
        return false;
    }

    // Check if the locale supports UTF-8
    if (strstr(locale, "UTF-8") != nullptr || strstr(locale, "UTF8") != nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}
