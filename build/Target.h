#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include "CompileUnit.h"
#include "BuildResult.h"
#include "ICompilable.h"

class Target : public ICompilable
{
public:
    // Constructor
    Target(const std::string& outputName, const std::string& compiler = "g++");

    // Setters
    void addCompileUnit(const std::shared_ptr<CompileUnit>& compileUnit);
    void addLibrary(const std::string& library);
    void addLibraryPath(const std::string& libraryPath);
    void addIncludePath(const std::string& includePath);
    void addCompileFlag(const std::string& flag);

    void addCompileFlags(const std::vector<std::string>& flags);
    void addLibraries(const std::vector<std::string>& libs);
    void addLibraryPaths(const std::vector<std::string>& paths);
    void addIncludePaths(const std::vector<std::string>& paths);

    // Getters
    const std::string& getOutputName() const;
    const std::vector<std::shared_ptr<CompileUnit>>& getCompileUnits() const;
    const std::vector<std::string>& getLibraries() const;
    const std::vector<std::string>& getLibraryPaths() const;
    const std::vector<std::string>& getIncludePaths() const;
    const std::vector<std::string>& getCompileFlags() const;
    const std::string& getCompiler() const;

    // Implementations of ICompilable
    virtual std::shared_ptr<BuildResult> build() override;
    virtual bool needsRecompilation() override;
    virtual bool canBuild() override;
    virtual std::string getName() const override;

protected:
    std::string outputName;                                 // Output name of the target
    std::string compiler;                                   // Compiler to use (e.g., g++, clang++)
    std::vector<std::shared_ptr<CompileUnit>> compileUnits; // Compile units included in this target
    std::vector<std::string> libraries;                     // Libraries to link against (e.g., -l)
    std::vector<std::string> libraryPaths;                  // Library directories (e.g., -L)
    std::vector<std::string> includePaths;                  // Include directories (e.g., -I)
    std::vector<std::string> compileFlags;                  // Additional compile flags

    // Helper Methods
    std::shared_ptr<BuildResult> linkObjects();
};
