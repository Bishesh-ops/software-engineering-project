#ifndef COMPILER_DRIVER_H
#define COMPILER_DRIVER_H

#include <string>
#include <vector>
#include <memory>

// ============================================================================
// Compiler Driver - End-to-End Compilation Pipeline
// ============================================================================
// Orchestrates the full compilation process from C source to executable binary
//
// Pipeline stages:
// 1. Lexical Analysis (Tokenization)
// 2. Parsing (AST Construction)
// 3. Semantic Analysis (Type Checking)
// 4. IR Generation (SSA Form)
// 5. Optimization (Constant Folding, Dead Code Elimination, etc.)
// 6. Code Generation (x86-64 Assembly)
// 7. Assembly (AS -> Object File)
// 8. Linking (LD -> Executable)

class CompilerDriver
{
public:
    // Compilation options
    struct Options {
        bool verbose = false;           // Print detailed compilation steps
        bool keepIntermediateFiles = false;  // Keep .s and .o files
        bool optimize = true;           // Run optimization passes
        std::string outputFile = "a.out";    // Output executable name
        bool emitAssembly = false;      // Stop after assembly generation
        bool emitObject = false;        // Stop after assembling
        bool linkWithCRT = true;        // Link with C runtime (for main)
        std::vector<std::string> linkLibraries;  // Additional libraries to link
    };

private:
    Options options;
    std::string tempDir;
    int errorCount;

    // Helper methods
    std::string getTempFileName(const std::string& baseName, const std::string& extension);
    bool runCommand(const std::string& command, std::string& output);
    bool writeAssemblyFile(const std::string& assembly, const std::string& filename);
    bool assembleFile(const std::string& asmFile, const std::string& objFile);
    bool linkFiles(const std::vector<std::string>& objFiles, const std::string& executable);
    void cleanup(const std::vector<std::string>& files);
    void reportError(const std::string& message);
    void reportInfo(const std::string& message);

public:
    CompilerDriver();
    explicit CompilerDriver(const Options& opts);
    ~CompilerDriver() = default;

    // Main compilation entry point
    // Returns true if compilation succeeded, false otherwise
    bool compile(const std::string& sourceCode, const std::string& sourceName = "input.c");

    // Compile multiple source files
    bool compileMultiple(const std::vector<std::string>& sourceFiles);

    // Get/Set options
    void setOptions(const Options& opts) { options = opts; }
    Options getOptions() const { return options; }

    // Get error count
    int getErrorCount() const { return errorCount; }

    // Static helper: Generate C runtime startup code for standalone executables
    static std::string generateStartupCode();
};

#endif // COMPILER_DRIVER_H
