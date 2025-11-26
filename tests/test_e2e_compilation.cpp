#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "ir_codegen.h"
#include "ir_optimizer.h"
#include "codegen.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

// ============================================================================
// End-to-End Compilation Test
// ============================================================================
// Tests the complete pipeline: C source -> Assembly -> Object -> Executable

bool runCommand(const std::string& cmd) {
    std::cout << "[CMD] " << cmd << std::endl;
    int status = system(cmd.c_str());
    return (status == 0);
}

bool compileAndTest(const std::string& testName, const std::string& sourceCode, int expectedExitCode) {
    std::cout << "\n========================================\n";
    std::cout << "Test: " << testName << "\n";
    std::cout << "========================================\n\n";

    // Compile
    Lexer lexer(sourceCode, "test.c");
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::unique_ptr<Program> ast = parser.parseProgram();
    if (parser.hasErrors()) {
        std::cerr << "[FAIL] Parsing failed\n";
        return false;
    }

    SemanticAnalyzer analyzer;
    analyzer.analyze(*ast);
    if (analyzer.hasErrors()) {
        std::cerr << "[FAIL] Semantic analysis failed\n";
        return false;
    }

    IRCodeGenerator irGen;
    irGen.generate(*ast);
    auto& functions = irGen.getFunctions();

    IROptimizer optimizer;
    for (auto& func : functions) {
        optimizer.optimize(func.get());
    }

    CodeGenerator codegen;
    std::string assembly = codegen.generateProgram(functions);

    // Write assembly
    std::string asmFile = "/tmp/test_e2e.s";
    std::ofstream outFile(asmFile);
    outFile << assembly;
    outFile.close();

    std::cout << "[INFO] Assembly generated (" << assembly.size() << " bytes)\n";

    // Assemble
    std::string objFile = "/tmp/test_e2e.o";
    if (!runCommand("as -o " + objFile + " " + asmFile + " 2>&1")) {
        std::cerr << "[FAIL] Assembly failed\n";
        return false;
    }

    // Link - Use gcc for easier linking with CRT
    std::string exeFile = "/tmp/test_e2e";
    if (!runCommand("gcc -o " + exeFile + " " + objFile + " 2>&1")) {
        std::cerr << "[FAIL] Linking failed\n";
        return false;
    }

    std::cout << "[INFO] Executable created: " << exeFile << "\n";

    // Run and check exit code
    int exitCode = system(exeFile.c_str());
    exitCode = WEXITSTATUS(exitCode);

    std::cout << "[INFO] Program exit code: " << exitCode << " (expected: " << expectedExitCode << ")\n";

    // Cleanup
    remove(asmFile.c_str());
    remove(objFile.c_str());
    remove(exeFile.c_str());

    if (exitCode == expectedExitCode) {
        std::cout << "[PASS] " << testName << "\n";
        return true;
    } else {
        std::cout << "[FAIL] " << testName << " - Wrong exit code\n";
        return false;
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "END-TO-END COMPILATION TESTS\n";
    std::cout << "========================================\n";

    int passed = 0;
    int total = 0;

    // ========================================================================
    // Test 1: Simple main returning constant
    // ========================================================================
    total++;
    if (compileAndTest(
        "Simple return",
        "int main() { return 42; }",
        42
    )) {
        passed++;
    }

    // ========================================================================
    // Test 2: Arithmetic in main
    // ========================================================================
    total++;
    if (compileAndTest(
        "Arithmetic",
        "int main() { int x = 10; int y = 20; return x + y; }",
        30
    )) {
        passed++;
    }

    // ========================================================================
    // Test 3: Function call
    // ========================================================================
    total++;
    if (compileAndTest(
        "Function call",
        R"(
            int add(int a, int b) {
                return a + b;
            }
            int main() {
                return add(15, 25);
            }
        )",
        40
    )) {
        passed++;
    }

    // ========================================================================
    // Test 4: Multiplication and subtraction
    // ========================================================================
    total++;
    if (compileAndTest(
        "Complex arithmetic",
        "int main() { int x = 5; int y = 3; return x * y - 1; }",
        14
    )) {
        passed++;
    }

    // ========================================================================
    // Summary
    // ========================================================================
    std::cout << "\n========================================\n";
    std::cout << "Test Results: " << passed << "/" << total << " passed\n";
    std::cout << "========================================\n";

    return (passed == total) ? 0 : 1;
}
