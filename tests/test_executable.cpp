#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic_analyzer.h"
#include "../include/ir_codegen.h"
#include "../include/ir_optimizer.h"
#include "../include/codegen.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sys/wait.h>

// ============================================================================
// Executable Generation Test
// ============================================================================
// Tests generating actual runnable executables

bool writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file) return false;
    file << content;
    return true;
}

bool runCommand(const std::string& cmd) {
    std::cout << "[CMD] " << cmd << std::endl;
    int status = system(cmd.c_str());
    return (status == 0);
}

int testCompilation(const std::string& testName, const std::string& sourceCode, int expectedExitCode) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << testName << "\n";
    std::cout << "========================================\n\n";

    // Pipeline
    Lexer lexer(sourceCode, "test.c");
    Parser parser(lexer);
    auto ast = parser.parseProgram();

    if (parser.hadError()) {
        std::cerr << "[FAIL] Parser errors\n";
        return 1;
    }

    SemanticAnalyzer analyzer;
    for (auto& decl : ast) {
        decl->accept(analyzer);
    }

    if (analyzer.has_errors()) {
        std::cerr << "[FAIL] Semantic errors\n";
        return 1;
    }

    IRCodeGenerator irGen;
    for (auto& decl : ast) {
        decl->accept(irGen);
    }

    auto& functions = irGen.getFunctions();

    IROptimizer optimizer;
    for (auto& func : functions) {
        optimizer.optimize(func.get());
    }

    CodeGenerator codegen;
    std::string assembly = codegen.generateProgram(functions);

    std::cout << "[INFO] Generated " << assembly.size() << " bytes of assembly\n";

    // Write assembly
    if (!writeFile("/tmp/test.s", assembly)) {
        std::cerr << "[FAIL] Could not write assembly\n";
        return 1;
    }

    // Assemble and link with gcc
    if (!runCommand("gcc -o /tmp/test_exe /tmp/test.s 2>&1")) {
        std::cerr << "[FAIL] Assembly/linking failed\n";
        return 1;
    }

    // Run executable
    int exitCode = system("/tmp/test_exe");
    exitCode = WEXITSTATUS(exitCode);

    std::cout << "[INFO] Exit code: " << exitCode << " (expected: " << expectedExitCode << ")\n";

    // Cleanup
    remove("/tmp/test.s");
    remove("/tmp/test_exe");

    if (exitCode == expectedExitCode) {
        std::cout << "[PASS] " << testName << "\n";
        return 0;
    } else {
        std::cout << "[FAIL] Wrong exit code\n";
        return 1;
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "EXECUTABLE GENERATION TESTS\n";
    std::cout << "========================================\n";
    std::cout << "User Story 4: Generate Executable Output File\n\n";

    int passed = 0;
    int total = 0;

    // Test 1
    total++;
    if (testCompilation(
        "Simple main returns 42",
        "int main() { return 42; }",
        42
    ) == 0) passed++;

    // Test 2
    total++;
    if (testCompilation(
        "Arithmetic in main",
        "int main() { int x = 10; int y = 20; return x + y; }",
        30
    ) == 0) passed++;

    // Test 3
    total++;
    if (testCompilation(
        "Function call",
        R"(
int add(int a, int b) {
    return a + b;
}

int main() {
    return add(7, 8);
}
        )",
        15
    ) == 0) passed++;

    // Summary
    std::cout << "\n========================================\n";
    std::cout << "Results: " << passed << "/" << total << " tests passed\n";
    std::cout << "========================================\n";

    return (passed == total) ? 0 : 1;
}
