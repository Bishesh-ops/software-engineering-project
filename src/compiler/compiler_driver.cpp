#include "compiler_driver.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "ir_codegen.h"
#include "ir_optimizer.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

// ============================================================================
// Compiler Driver Implementation
// ============================================================================

CompilerDriver::CompilerDriver()
    : errorCount(0)
{
    // Use system temp directory
    const char* tmpDir = getenv("TMPDIR");
    if (tmpDir) {
        tempDir = tmpDir;
    } else {
        tempDir = "/tmp";
    }
}

CompilerDriver::CompilerDriver(const Options& opts)
    : options(opts), errorCount(0)
{
    const char* tmpDir = getenv("TMPDIR");
    if (tmpDir) {
        tempDir = tmpDir;
    } else {
        tempDir = "/tmp";
    }
}

std::string CompilerDriver::getTempFileName(const std::string& baseName, const std::string& extension)
{
    return tempDir + "/" + baseName + extension;
}

void CompilerDriver::reportError(const std::string& message)
{
    std::cerr << "Error: " << message << std::endl;
    errorCount++;
}

void CompilerDriver::reportInfo(const std::string& message)
{
    if (options.verbose) {
        std::cout << "[INFO] " << message << std::endl;
    }
}

bool CompilerDriver::runCommand(const std::string& command, std::string& output)
{
    if (options.verbose) {
        std::cout << "[CMD] " << command << std::endl;
    }

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        reportError("Failed to execute command: " + command);
        return false;
    }

    char buffer[256];
    output.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    int status = pclose(pipe);
    if (status != 0) {
        reportError("Command failed with status " + std::to_string(status));
        if (!output.empty()) {
            std::cerr << output << std::endl;
        }
        return false;
    }

    return true;
}

bool CompilerDriver::writeAssemblyFile(const std::string& assembly, const std::string& filename)
{
    reportInfo("Writing assembly to: " + filename);

    std::ofstream outFile(filename);
    if (!outFile) {
        reportError("Failed to open file for writing: " + filename);
        return false;
    }

    outFile << assembly;
    outFile.close();

    if (options.verbose) {
        std::cout << "[INFO] Assembly file size: " << assembly.size() << " bytes\n";
    }

    return true;
}

bool CompilerDriver::assembleFile(const std::string& asmFile, const std::string& objFile)
{
    reportInfo("Assembling: " + asmFile + " -> " + objFile);

    // Use system assembler (as)
    std::string command = "as -o " + objFile + " " + asmFile + " 2>&1";
    std::string output;

    if (!runCommand(command, output)) {
        reportError("Assembly failed");
        return false;
    }

    reportInfo("Assembly successful");
    return true;
}

bool CompilerDriver::linkFiles(const std::vector<std::string>& objFiles, const std::string& executable)
{
    reportInfo("Linking: " + std::to_string(objFiles.size()) + " object file(s) -> " + executable);

    // Build link command
    std::ostringstream cmd;

#ifdef __APPLE__
    // macOS linker flags
    if (options.linkWithCRT) {
        // Link with system libraries for main()
        cmd << "ld -o " << executable << " ";
        cmd << "-lSystem ";  // System library (libc, etc.)
        cmd << "-syslibroot $(xcrun --show-sdk-path) ";  // SDK path
        cmd << "-arch arm64 ";  // Architecture (for Apple Silicon)
        cmd << "-e _main ";  // Entry point
    } else {
        // Standalone executable with custom _start
        cmd << "ld -o " << executable << " ";
        cmd << "-e _start ";
        cmd << "-arch arm64 ";
    }
#else
    // Linux linker flags
    if (options.linkWithCRT) {
        // Use GCC for linking (easier than ld directly)
        cmd << "gcc -o " << executable << " ";
    } else {
        // Use ld directly
        cmd << "ld -o " << executable << " ";
        cmd << "-e _start ";
    }
#endif

    // Add object files
    for (const auto& objFile : objFiles) {
        cmd << objFile << " ";
    }

    // Add additional libraries
    for (const auto& lib : options.linkLibraries) {
        cmd << "-l" << lib << " ";
    }

    cmd << "2>&1";

    std::string output;
    if (!runCommand(cmd.str(), output)) {
        reportError("Linking failed");
        return false;
    }

    reportInfo("Linking successful");
    return true;
}

void CompilerDriver::cleanup(const std::vector<std::string>& files)
{
    if (options.keepIntermediateFiles) {
        reportInfo("Keeping intermediate files");
        return;
    }

    reportInfo("Cleaning up intermediate files");
    for (const auto& file : files) {
        if (remove(file.c_str()) != 0 && options.verbose) {
            std::cerr << "[WARN] Failed to remove: " << file << std::endl;
        }
    }
}

bool CompilerDriver::compile(const std::string& sourceCode, const std::string& sourceName)
{
    errorCount = 0;

    reportInfo("========================================");
    reportInfo("C Compiler - Full Compilation Pipeline");
    reportInfo("========================================");
    reportInfo("Source: " + sourceName);
    reportInfo("Output: " + options.outputFile);
    reportInfo("");

    // ========================================================================
    // Stage 1: Lexical Analysis
    // ========================================================================
    reportInfo("Stage 1: Lexical Analysis");

    Lexer lexer(sourceCode, sourceName);
    std::vector<Token> tokens = lexer.tokenize();

    if (tokens.empty() || tokens.back().type != TokenType::END_OF_FILE) {
        reportError("Lexical analysis failed");
        return false;
    }

    reportInfo("  -> " + std::to_string(tokens.size()) + " tokens generated");

    // ========================================================================
    // Stage 2: Parsing
    // ========================================================================
    reportInfo("Stage 2: Parsing (AST Construction)");

    Parser parser(tokens);
    std::unique_ptr<Program> ast = parser.parseProgram();

    if (!ast || parser.hasErrors()) {
        reportError("Parsing failed");
        const auto& errors = parser.getErrors();
        for (const auto& err : errors) {
            std::cerr << "  " << err << std::endl;
        }
        return false;
    }

    reportInfo("  -> AST constructed successfully");

    // ========================================================================
    // Stage 3: Semantic Analysis
    // ========================================================================
    reportInfo("Stage 3: Semantic Analysis");

    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.set_warnings_enabled(options.warningsEnabled);
    semanticAnalyzer.analyze(*ast);

    if (semanticAnalyzer.hasErrors()) {
        reportError("Semantic analysis failed");
        const auto& errors = semanticAnalyzer.getErrors();
        for (const auto& err : errors) {
            std::cerr << "  " << err << std::endl;
        }
        return false;
    }

    reportInfo("  -> Semantic analysis passed");

    // ========================================================================
    // Stage 4: IR Generation
    // ========================================================================
    reportInfo("Stage 4: IR Generation (SSA Form)");

    IRCodeGenerator irGen;
    irGen.generate(*ast);
    auto& functions = irGen.getFunctions();

    reportInfo("  -> " + std::to_string(functions.size()) + " function(s) generated");

    // ========================================================================
    // Stage 5: Optimization (Optional)
    // ========================================================================
    if (options.optimize) {
        reportInfo("Stage 5: Optimization");

        IROptimizer optimizer;
        for (auto& func : functions) {
            optimizer.optimize(func.get());
        }

        reportInfo("  -> Optimization passes completed");
    }

    // ========================================================================
    // Stage 6: Code Generation
    // ========================================================================
    reportInfo("Stage 6: Code Generation (x86-64 Assembly)");

    CodeGenerator codeGen;
    std::string assembly = codeGen.generateProgram(functions);

    reportInfo("  -> " + std::to_string(assembly.size()) + " bytes of assembly generated");

    // ========================================================================
    // Stage 7: Write Assembly File
    // ========================================================================
    std::string asmFile = getTempFileName(sourceName, ".s");
    if (options.emitAssembly) {
        asmFile = options.outputFile;  // User wants .s file as output
    }

    if (!writeAssemblyFile(assembly, asmFile)) {
        return false;
    }

    if (options.emitAssembly) {
        reportInfo("Assembly file written: " + asmFile);
        reportInfo("========================================");
        reportInfo("Compilation successful (assembly only)");
        reportInfo("========================================");
        return true;
    }

    // ========================================================================
    // Stage 8: Assembly (AS)
    // ========================================================================
    reportInfo("Stage 7: Assembling");

    std::string objFile = getTempFileName(sourceName, ".o");
    if (options.emitObject) {
        objFile = options.outputFile;  // User wants .o file as output
    }

    if (!assembleFile(asmFile, objFile)) {
        return false;
    }

    if (options.emitObject) {
        reportInfo("Object file written: " + objFile);
        cleanup({asmFile});
        reportInfo("========================================");
        reportInfo("Compilation successful (object only)");
        reportInfo("========================================");
        return true;
    }

    // ========================================================================
    // Stage 9: Linking (LD)
    // ========================================================================
    reportInfo("Stage 8: Linking");

    if (!linkFiles({objFile}, options.outputFile)) {
        return false;
    }

    // Make executable
#ifndef _WIN32
    chmod(options.outputFile.c_str(), 0755);
#endif

    // ========================================================================
    // Cleanup
    // ========================================================================
    cleanup({asmFile, objFile});

    reportInfo("");
    reportInfo("========================================");
    reportInfo("Compilation successful!");
    reportInfo("Executable: " + options.outputFile);
    reportInfo("========================================");

    return true;
}

bool CompilerDriver::compileMultiple(const std::vector<std::string>& sourceFiles)
{
    // TODO: Implement multi-file compilation
    reportError("Multi-file compilation not yet implemented");
    return false;
}

std::string CompilerDriver::generateStartupCode()
{
    // Generate minimal _start function for standalone executables
    std::ostringstream code;

    code << "# C Runtime Startup Code\n";
    code << "# Minimal _start function for standalone executables\n";
    code << "\n";
    code << ".text\n";
    code << ".globl _start\n";
    code << "_start:\n";
    code << "    # Set up stack frame\n";
    code << "    pushq %rbp\n";
    code << "    movq %rsp, %rbp\n";
    code << "\n";
    code << "    # Call main function\n";
    code << "    call main\n";
    code << "\n";
    code << "    # Exit with main's return value (in %rax)\n";
    code << "    movq %rax, %rdi    # exit code = return value\n";
#ifdef __APPLE__
    code << "    movq $0x2000001, %rax  # macOS exit syscall\n";
#else
    code << "    movq $60, %rax     # Linux exit syscall\n";
#endif
    code << "    syscall\n";

    return code.str();
}
