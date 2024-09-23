#include "CompileUnit.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <format>
#include <array>

namespace fs = std::filesystem;

CompileUnit::CompileUnit(const std::string& sourcePath, const std::string& outputPath,
                         const std::vector<std::string>& includePaths)
    : sourcePath(sourcePath), outputPath(outputPath), includePaths(includePaths), compiler("g++")
{
    // Parse header dependencies upon creation
    parseHeaderDependencies();
}

void CompileUnit::addDependency(CompileUnit* dependency)
{
    dependencies.push_back(dependency);
}

void CompileUnit::addCompileFlag(const std::string& flag)
{
    compileFlags.push_back(flag);
}

void CompileUnit::addIncludePath(const std::string& includePath)
{
    includePaths.push_back(includePath);
}

void CompileUnit::addCompileFlags(const std::vector<std::string>& flags)
{
    compileFlags.insert(compileFlags.end(), flags.begin(), flags.end());
}

void CompileUnit::addIncludePaths(const std::vector<std::string>& paths)
{
    includePaths.insert(includePaths.end(), paths.begin(), paths.end());
}

void CompileUnit::setCompiler(const std::string& compiler)
{
    this->compiler = compiler;
}

const std::string& CompileUnit::getSourcePath() const
{
    return sourcePath;
}

const std::string& CompileUnit::getOutputPath() const
{
    return outputPath;
}

const std::vector<CompileUnit*>& CompileUnit::getDependencies() const
{
    return dependencies;
}

const std::unordered_set<std::string>& CompileUnit::getHeaderDependencies() const
{
    return headerDependencies;
}

const std::vector<std::string>& CompileUnit::getCompileFlags() const
{
    return compileFlags;
}

const std::vector<std::string>& CompileUnit::getIncludePaths() const
{
    return includePaths;
}

const std::string& CompileUnit::getCompiler() const
{
    return compiler;
}

bool CompileUnit::needsRecompilation()
{
    // Get the last modified time of the output file
    std::time_t outputModTime = getLastModifiedTime(outputPath);

    // If the output file doesn't exist, we need to compile
    if (outputModTime == 0)
    {
        return true;
    }

    // Check if the source file has been modified since the output file was created
    std::time_t sourceModTime = getLastModifiedTime(sourcePath);
    if (sourceModTime > outputModTime)
    {
        return true;
    }

    // Check dependencies (other compile units)
    for (const auto& dep : dependencies)
    {
        if (dep->needsRecompilation())
        {
            return true;
        }

        // Get the last modified time of the dependency's output file
        std::time_t depOutputModTime = getLastModifiedTime(dep->getOutputPath());
        if (depOutputModTime > outputModTime)
        {
            return true;
        }
    }

    // Check header files
    for (const auto& header : headerDependencies)
    {
        std::time_t headerModTime = getLastModifiedTime(header);
        if (headerModTime > outputModTime)
        {
            return true;
        }
    }

    // No need to recompile
    return false;
}

void CompileUnit::parseHeaderDependencies()
{
    std::unordered_set<std::string> visited;
    parseHeadersRecursive(sourcePath, visited);
}

void CompileUnit::parseHeadersRecursive(const std::string& filePath, std::unordered_set<std::string>& visited)
{
    // Avoid processing the same file multiple times
    if (visited.find(filePath) != visited.end())
    {
        return;
    }

    visited.insert(filePath);

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        // Handle error: file not found or cannot be opened
        // For angle-bracket includes, we skip silently
        return;
    }

    std::string line;
    std::string currentDir = fs::path(filePath).parent_path().string();

    while (std::getline(file, line))
    {
        // Remove comments
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos);
        }

        // Remove leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Check if the line is an include directive
        if (line.substr(0, 8) == "#include")
        {
            std::istringstream iss(line);
            std::string directive, includePath;

            iss >> directive >> includePath;

            // Remove any trailing comments
            size_t commentStart = includePath.find("//");
            if (commentStart != std::string::npos)
            {
                includePath = includePath.substr(0, commentStart);
            }

            // Remove quotes or angle brackets
            if (includePath.size() >= 2)
            {
                char firstChar = includePath.front();
                char lastChar  = includePath.back();
                bool isQuotedInclude = false;
                bool isAngleBracketInclude = false;

                if (firstChar == '\"' && lastChar == '\"')
                {
                    isQuotedInclude = true;
                    includePath = includePath.substr(1, includePath.size() - 2);
                }
                else if (firstChar == '<' && lastChar == '>')
                {
                    isAngleBracketInclude = true;
                    includePath = includePath.substr(1, includePath.size() - 2);
                }
                else
                {
                    // Malformed include, skip
                    continue;
                }

                std::string foundPath;
                bool headerFound = findHeaderFile(includePath, currentDir, isAngleBracketInclude, foundPath);

                if (headerFound)
                {
                    // Add the header to the dependencies
                    headerDependencies.insert(foundPath);

                    // Recursively parse the included header
                    parseHeadersRecursive(foundPath, visited);
                }
                else
                {
                    // Skip silently if angle-bracket include not found
                    // For quoted includes, you might want to log an error
                    if (isQuotedInclude)
                    {
                        std::cerr << "Warning: Header file \"" << includePath << "\" not found." << std::endl;
                    }
                }
            }
        }
    }
}

bool CompileUnit::findHeaderFile(const std::string& headerName, const std::string& currentDir,
                                 bool isAngleBracketInclude, std::string& foundPath)
{
    // For quoted includes, search the current directory first
    if (!isAngleBracketInclude)
    {
        fs::path potentialPath = fs::path(currentDir) / headerName;
        if (fs::exists(potentialPath))
        {
            foundPath = potentialPath.string();
            return true;
        }
    }

    // Search in include paths
    for (const auto& dir : includePaths)
    {
        fs::path potentialPath = fs::path(dir) / headerName;
        if (fs::exists(potentialPath))
        {
            foundPath = potentialPath.string();
            return true;
        }
    }

    // For angle-bracket includes, skip silently if not found
    // For quoted includes, return false if not found
    return false;
}

std::time_t CompileUnit::getLastModifiedTime(const std::string& filePath) const
{
    struct stat result;
    if (stat(filePath.c_str(), &result) == 0)
    {
        return result.st_mtime;
    }
    else
    {
        // File does not exist
        return 0;
    }
}

std::shared_ptr<BuildResult> CompileUnit::build()
{
    auto result = std::make_shared<BuildResult>();
    result->success = true;

    // Build the compile command
    std::ostringstream command;
    command << compiler << " -c \"" << sourcePath << "\" -o \"" << outputPath << "\"";

    // Add include paths
    for (const auto& includePath : includePaths)
    {
        command << " -I\"" << includePath << "\"";
    }

    // Add compile flags
    for (const auto& flag : compileFlags)
    {
        command << " " << flag;
    }

    // Execute the compile command and capture output
    std::string cmdStr = std::format("mkdir -p \"$(dirname \"{}\")\" && {}",
                                     outputPath,
                                     command.str());
    std::array<char, 128> buffer;
    std::string output = cmdStr + "\n";

    // Open a pipe to read the output
    FILE* pipe = popen((cmdStr + " 2>&1").c_str(), "r");
    if (!pipe)
    {
        result->success = false;
        result->errors.push_back("Failed to execute compile command.");
        return result;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        output += buffer.data();
    }

    int returnCode = pclose(pipe);

    result->output = output;

    // Parse warnings and errors from the output
    std::istringstream outputStream(output);
    std::string line;
    while (std::getline(outputStream, line))
    {
        if (line.find("warning:") != std::string::npos)
        {
            result->warnings.push_back(line);
        }
        else if (line.find("error:") != std::string::npos)
        {
            result->errors.push_back(line);
        }
    }

    if (returnCode != 0)
    {
        result->success = false;
    }

    return result;
}

bool CompileUnit::canBuild()
{
    // A CompileUnit can always attempt to build itself, as it does not depend on other compilables
    // For dependencies, ensure they are built first
    for (const auto& dep : dependencies)
    {
        if (!dep->canBuild())
        {
            return false;
        }
    }
    return true;
}

std::string CompileUnit::getName() const
{
    return sourcePath;
}
