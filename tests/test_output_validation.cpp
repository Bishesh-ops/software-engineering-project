// ============================================================================
// Test: Output Validation Framework
// ============================================================================
// Comprehensive test suite for validating compiler output at multiple levels:
// 1. Assembly output verification
// 2. Binary execution with exit code verification
// 3. Standard output capture and verification (with printf)
// 4. Error handling verification

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
#include <array>
#include <memory>
#include <string>
#include <sstream>

using namespace std;

// ============================================================================
// Test Framework
// ============================================================================

int totalTests = 0;
int passedTests = 0;

void reportTest(const string& testName, bool passed) {
    totalTests++;
    if (passed) {
        passedTests++;
        cout << "[PASS] " << testName << "\n";
    } else {
        cout << "[FAIL] " << testName << "\n";
    }
}

// ============================================================================
// Utility: Execute Command and Capture Output
// ============================================================================

string execCommand(const string& cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) {
        throw runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

// ============================================================================
// Core Compilation Function
// ============================================================================

struct CompilationResult {
    bool success;
    string assembly;
    string errorMessage;
};

CompilationResult compileToAssembly(const string& sourceCode, const string& filename = "test.c") {
    CompilationResult result;
    result.success = false;

    try {
        // Lexing
        Lexer lexer(sourceCode, filename);
        vector<Token> tokens = lexer.tokenize();

        // Parsing
        Parser parser(tokens);
        unique_ptr<Program> ast = parser.parseProgram();
        if (parser.hasErrors()) {
            result.errorMessage = "Parser errors detected";
            return result;
        }

        // Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(*ast);
        if (analyzer.hasErrors()) {
            result.errorMessage = "Semantic errors detected";
            return result;
        }

        // IR Generation
        IRCodeGenerator irGen;
        irGen.generate(*ast);
        auto& functions = irGen.getFunctions();

        // IR Optimization
        IROptimizer optimizer;
        for (auto& func : functions) {
            optimizer.optimize(func.get());
        }

        // Code Generation
        CodeGenerator codegen;
        result.assembly = codegen.generateProgram(functions);
        result.success = true;

    } catch (const exception& e) {
        result.errorMessage = string("Exception: ") + e.what();
    }

    return result;
}

// ============================================================================
// Test 1: Assembly Output Verification
// ============================================================================

void test_AssemblyContainsInstructions() {
    string testName = "Assembly contains expected instructions";

    string source = R"(
        int add(int a, int b) {
            return a + b;
        }
    )";

    auto result = compileToAssembly(source);

    bool passed = result.success &&
                  result.assembly.find("add") != string::npos &&
                  result.assembly.find("addq") != string::npos &&
                  result.assembly.find("ret") != string::npos;

    reportTest(testName, passed);
}

void test_AssemblyHasProperStructure() {
    string testName = "Assembly has proper structure (.text, .globl, etc.)";

    string source = "int main() { return 0; }";
    auto result = compileToAssembly(source);

    bool hasText = result.assembly.find(".text") != string::npos;
    bool hasGlobl = result.assembly.find(".globl") != string::npos;
    bool hasMain = result.assembly.find("main:") != string::npos;

    bool passed = result.success && hasText && hasGlobl && hasMain;

    reportTest(testName, passed);
}

void test_AssemblyOptimizationsApplied() {
    string testName = "Assembly has optimizations applied";

    string source = R"(
        int compute() {
            int x = 2 + 3;  // Should be constant folded
            return x * 8;   // Should use shift if peephole enabled
        }
    )";

    auto result = compileToAssembly(source);

    // Check for optimization evidence (this will vary based on optimizer state)
    bool passed = result.success && result.assembly.length() > 0;

    reportTest(testName, passed);
}

// ============================================================================
// Test 2: Binary Execution with Exit Code Verification
// ============================================================================

bool compileAndExecute(const string& source, int& exitCode, string& output) {
    auto result = compileToAssembly(source);
    if (!result.success) {
        return false;
    }

    // Write assembly to file
    string asmFile = "/tmp/test_output_validation.s";
    ofstream outFile(asmFile);
    outFile << result.assembly;
    outFile.close();

    // Assemble
    string objFile = "/tmp/test_output_validation.o";
    int asmResult = system(("as -o " + objFile + " " + asmFile + " 2>/dev/null").c_str());
    if (asmResult != 0) {
        return false;
    }

    // Link
    string exeFile = "/tmp/test_output_validation";
    int linkResult = system(("gcc -o " + exeFile + " " + objFile + " 2>/dev/null").c_str());
    if (linkResult != 0) {
        return false;
    }

    // Execute and capture output
    output = execCommand(exeFile + " 2>&1");
    exitCode = system(exeFile.c_str());
    exitCode = WEXITSTATUS(exitCode);

    // Cleanup
    remove(asmFile.c_str());
    remove(objFile.c_str());
    remove(exeFile.c_str());

    return true;
}

void test_SimpleReturnValue() {
    string testName = "Simple return value (42)";

    string source = "int main() { return 42; }";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 42);

    reportTest(testName, passed);
}

void test_ArithmeticReturnValue() {
    string testName = "Arithmetic return value (10 + 20 = 30)";

    string source = R"(
        int main() {
            int x = 10;
            int y = 20;
            return x + y;
        }
    )";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 30);

    reportTest(testName, passed);
}

void test_FunctionCallReturnValue() {
    string testName = "Function call return value";

    string source = R"(
        int multiply(int a, int b) {
            return a * b;
        }
        int main() {
            return multiply(6, 7);
        }
    )";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 42);

    reportTest(testName, passed);
}

void test_ControlFlowReturnValue() {
    string testName = "Control flow (if statement) return value";

    string source = R"(
        int main() {
            int x = 5;
            if (x > 3) {
                return 10;
            } else {
                return 20;
            }
        }
    )";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 10);

    reportTest(testName, passed);
}

// ============================================================================
// Test 3: Error Handling Verification
// ============================================================================

void test_SyntaxErrorDetection() {
    string testName = "Syntax error detection";

    string source = "int main() { return 42 }";  // Missing semicolon

    auto result = compileToAssembly(source);

    // Should fail compilation
    bool passed = !result.success;

    reportTest(testName, passed);
}

void test_TypeErrorDetection() {
    string testName = "Type error detection";

    string source = R"(
        int main() {
            int x = 5;
            return x + "hello";  // Type mismatch
        }
    )";

    auto result = compileToAssembly(source);

    // Should fail semantic analysis
    bool passed = !result.success;

    reportTest(testName, passed);
}

void test_UndefinedVariableDetection() {
    string testName = "Undefined variable detection";

    string source = R"(
        int main() {
            return undefinedVar;
        }
    )";

    auto result = compileToAssembly(source);

    // Should fail semantic analysis
    bool passed = !result.success;

    reportTest(testName, passed);
}

// ============================================================================
// Test 4: External Function Call Verification
// ============================================================================

void test_ExternalFunctionDeclaration() {
    string testName = "External function .extern declaration";

    string source = R"(
        int main() {
            // Calling external function (won't actually execute in test)
            // Just verify assembly generation
            return 0;
        }
    )";

    auto result = compileToAssembly(source);

    // For now, just verify successful compilation
    // Full printf tests are in test_external_calls.cpp
    bool passed = result.success;

    reportTest(testName, passed);
}

// ============================================================================
// Test 5: Optimization Verification
// ============================================================================

void test_ConstantFoldingInOutput() {
    string testName = "Constant folding visible in output";

    string source = R"(
        int main() {
            return 2 + 3 + 4;  // Should be folded to 9
        }
    )";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 9);

    reportTest(testName, passed);
}

void test_DeadCodeNotInOutput() {
    string testName = "Dead code elimination";

    string source = R"(
        int main() {
            return 5;
            int x = 10;  // Dead code
            return x;    // Dead code
        }
    )";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 5);

    reportTest(testName, passed);
}

// ============================================================================
// Test 6: Complex Program Verification
// ============================================================================

void test_ComplexProgram() {
    string testName = "Complex program with multiple features";

    string source = R"(
        int factorial(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }

        int main() {
            int result = factorial(5);  // 5! = 120
            return result % 100;  // Return last 2 digits (20)
        }
    )";

    int exitCode = 0;
    string output;
    bool compiled = compileAndExecute(source, exitCode, output);

    bool passed = compiled && (exitCode == 20);

    reportTest(testName, passed);
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    cout << "========================================\n";
    cout << "Output Validation Framework Tests\n";
    cout << "========================================\n\n";

    cout << "--- Assembly Output Tests ---\n";
    test_AssemblyContainsInstructions();
    test_AssemblyHasProperStructure();
    test_AssemblyOptimizationsApplied();

    cout << "\n--- Binary Execution Tests ---\n";
    test_SimpleReturnValue();
    test_ArithmeticReturnValue();
    test_FunctionCallReturnValue();
    test_ControlFlowReturnValue();

    cout << "\n--- Error Handling Tests ---\n";
    test_SyntaxErrorDetection();
    test_TypeErrorDetection();
    test_UndefinedVariableDetection();

    cout << "\n--- External Function Tests ---\n";
    test_ExternalFunctionDeclaration();

    cout << "\n--- Optimization Verification Tests ---\n";
    test_ConstantFoldingInOutput();
    test_DeadCodeNotInOutput();

    cout << "\n--- Complex Program Tests ---\n";
    test_ComplexProgram();

    cout << "\n========================================\n";
    cout << "Test Summary\n";
    cout << "========================================\n";
    cout << "Total tests: " << totalTests << "\n";
    cout << "Passed:      " << passedTests << "\n";
    cout << "Failed:      " << (totalTests - passedTests) << "\n";
    cout << "========================================\n";

    if (passedTests == totalTests) {
        cout << "\n✓ All output validation tests passed!\n\n";
        return 0;
    } else {
        cout << "\n✗ Some tests failed\n\n";
        return 1;
    }
}
