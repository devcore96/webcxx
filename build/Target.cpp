#include "Target.h"
#include <iostream>
#include <filesystem>
#include <sstream>
#include <cstdio>
#include <format>
#include <array>

namespace fs = std::filesystem;

Target::Target(const std::string& outputName, const std::string& compiler)
    : outputName(outputName), compiler(compiler) { }

void Target::addCompileUnit(const std::shared_ptr<CompileUnit>& compileUnit)
{
    // Add the target's include paths and compile flags to the compile unit
    for (const auto& includePath : includePaths)
    {
        compileUnit->addIncludePath(includePath);
    }

    for (const auto& flag : compileFlags)
    {
        compileUnit->addCompileFlag(flag);
    }

    // Set the compiler for the compile unit
    compileUnit->setCompiler(compiler);

    compileUnits.push_back(compileUnit);
}

void Target::addLibrary(const std::string& library)
{
    libraries.push_back(library);
}

void Target::addLibraryPath(const std::string& libraryPath)
{
    libraryPaths.push_back(libraryPath);
}

void Target::addIncludePath(const std::string& includePath)
{
    includePaths.push_back(includePath);
}

void Target::addCompileFlag(const std::string& flag)
{
    compileFlags.push_back(flag);
}

void Target::addCompileFlags(const std::vector<std::string>& flags)
{
    compileFlags.insert(compileFlags.end(), flags.begin(), flags.end());
}

void Target::addLibraries(const std::vector<std::string>& libs)
{
    libraries.insert(libraries.end(), libs.begin(), libs.end());
}

void Target::addLibraryPaths(const std::vector<std::string>& paths)
{
    libraryPaths.insert(libraryPaths.end(), paths.begin(), paths.end());
}

void Target::addIncludePaths(const std::vector<std::string>& paths)
{
    includePaths.insert(includePaths.end(), paths.begin(), paths.end());
}

const std::string& Target::getOutputName() const
{
    return outputName;
}

const std::vector<std::shared_ptr<CompileUnit>>& Target::getCompileUnits() const
{
    return compileUnits;
}

const std::vector<std::string>& Target::getLibraries() const
{
    return libraries;
}

const std::vector<std::string>& Target::getLibraryPaths() const
{
    return libraryPaths;
}

const std::vector<std::string>& Target::getIncludePaths() const
{
    return includePaths;
}

const std::vector<std::string>& Target::getCompileFlags() const
{
    return compileFlags;
}

const std::string& Target::getCompiler() const
{
    return compiler;
}

bool Target::needsRecompilation()
{
    // Check if the target output file exists
    if (!fs::exists(outputName))
    {
        return true;
    }

    // Check if any object file is newer than the target output
    auto targetModTime = fs::last_write_time(outputName);

    for (const auto& unit : compileUnits)
    {
        std::string objectFile = unit->getOutputPath();
        if (fs::exists(objectFile))
        {
            auto objModTime = fs::last_write_time(objectFile);
            if (objModTime > targetModTime)
            {
                return true;
            }
        }
        else
        {
            // Object file doesn't exist, need to rebuild
            return true;
        }
    }

    // No need to rebuild
    return false;
}

bool Target::canBuild()
{
    // Check if all compile units are ready (i.e., their object files exist)
    for (const auto& unit : compileUnits)
    {
        if (!fs::exists(unit->getOutputPath()))
        {
            // The object file is not available yet
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<BuildResult> Target::build()
{
    if (!canBuild())
    {
        throw std::runtime_error("Cannot build target '" + outputName + "' yet. Dependencies are not met.");
    }

    auto result = std::make_shared<BuildResult>();
    result->success = true;

    // Link object files into the final target
    auto linkResult = linkObjects();

    // Aggregate link results
    result->output  += linkResult->output;
    result->warnings = linkResult->warnings;
    result->errors   = linkResult->errors;
    result->success  = linkResult->success;

    return result;
}

std::shared_ptr<BuildResult> Target::linkObjects()
{
    auto result = std::make_shared<BuildResult>();
    result->success = true;

    // Build the link command
    std::ostringstream command;
    command << compiler << " -o \"" << outputName << "\"";

    // Add object files
    for (const auto& unit : compileUnits)
    {
        command << " \"" << unit->getOutputPath() << "\"";
    }

    // Add library paths
    for (const auto& libPath : libraryPaths)
    {
        command << " -L\"" << libPath << "\"";
    }

    // Add libraries
    for (const auto& lib : libraries)
    {
        command << " -l" << lib;
    }

    // Add additional compile flags
    for (const auto& flag : compileFlags)
    {
        command << " " << flag;
    }

    // Execute the link command and capture output
    std::string cmdStr = std::format("mkdir -p \"$(dirname \"{}\")\" && {}",
                                     outputName,
                                     command.str());
    std::array<char, 128> buffer;
    std::string output = cmdStr + "\n";

    // Open a pipe to read the output
    FILE* pipe = popen((cmdStr + " 2>&1").c_str(), "r");
    if (!pipe)
    {
        result->success = false;
        result->errors.push_back("Failed to execute linker command.");
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

std::string Target::getName() const
{
    return outputName;
}
