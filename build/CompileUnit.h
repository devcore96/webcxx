#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <ctime>
#include <memory>
#include "BuildResult.h"
#include "ICompilable.h"

class CompileUnit : public ICompilable
{
public:
    CompileUnit(const std::string& sourcePath, const std::string& outputPath,
                const std::vector<std::string>& includePaths = {});

    // Setters
    void addDependency(CompileUnit* dependency);
    void addCompileFlag(const std::string& flag);
    void addIncludePath(const std::string& includePath);

    void addCompileFlags(const std::vector<std::string>& flags);
    void addIncludePaths(const std::vector<std::string>& paths);

    void setCompiler(const std::string& compiler);

    // Getters
    const std::string& getSourcePath() const;
    const std::string& getOutputPath() const;
    const std::vector<CompileUnit*>& getDependencies() const;
    const std::unordered_set<std::string>& getHeaderDependencies() const;
    const std::vector<std::string>& getCompileFlags() const;
    const std::vector<std::string>& getIncludePaths() const;
    const std::string& getCompiler() const;

    // Implementations of ICompilable
    virtual std::shared_ptr<BuildResult> build() override;
    virtual bool needsRecompilation() override;
    virtual bool canBuild() override;
    virtual std::string getName() const override;

    // Other Methods
    void parseHeaderDependencies();

private:
    std::string sourcePath;                             // Relative path to the source file
    std::string outputPath;                             // Relative path for the output file
    std::vector<CompileUnit*> dependencies;             // Other compile units this unit depends on
    std::unordered_set<std::string> headerDependencies; // Header files this unit depends on
    std::vector<std::string> compileFlags;              // Additional compile flags
    std::vector<std::string> includePaths;              // Include paths for headers
    std::string compiler;                               // Compiler to use (e.g., g++, clang++)

    // Helper methods
    void parseHeadersRecursive(const std::string& filePath, std::unordered_set<std::string>& visited);
    bool findHeaderFile(const std::string& headerName, const std::string& currentDir, bool isAngleBracketInclude, std::string& foundPath);
    std::time_t getLastModifiedTime(const std::string& filePath) const;
};
