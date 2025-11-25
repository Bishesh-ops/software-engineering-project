#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic_analyzer.h"
#include <iostream>
#include <string>
#include <vector>

// Integration Test - Full Pipeline: Lexer → Parser → Semantic Analyzer
// Tests the complete compilation flow from source code to validated AST

void printTestHeader(const std::string& title) {
    std::cout << "\n========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n";
}

void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << "\n";
}

// Test helper: Run full pipeline on source code
struct TestResult {
    bool lexer_success;
    bool parser_success;
    bool semantic_success;
    std::vector<std::string> errors;
};

TestResult runFullPipeline(const std::string& sourceCode, const std::string& filename = "test.c") {
    TestResult result;
    result.lexer_success = false;
    result.parser_success = false;
    result.semantic_success = false;

    // STAGE 1: Lexer - Create lexer with source code
    Lexer lexer(sourceCode, filename);
    result.lexer_success = true; // Lexer construction succeeded

    // STAGE 2: Parser - Build AST from lexer
    Parser parser(lexer);
    std::vector<std::unique_ptr<Declaration>> ast;

    try {
        // Parse the entire program (all top-level declarations)
        ast = parser.parseProgram();

        // Check for parser errors
        if (parser.hadError()) {
            for (const auto& error : parser.getErrors()) {
                result.errors.push_back("Parser error: " + error.message);
            }
            return result;
        }

        result.parser_success = true;
    } catch (const std::exception& e) {
        result.errors.push_back(std::string("Parser error: ") + e.what());
        return result;
    }

    // STAGE 3: Semantic Analyzer - Validate AST
    SemanticAnalyzer analyzer;

    try {
        // Analyze all declarations
        for (auto& decl : ast) {
            decl->accept(analyzer);
        }

        // Check for semantic errors
        if (analyzer.has_errors()) {
            for (const auto& error : analyzer.get_errors()) {
                result.errors.push_back("Semantic error: " + error.message);
            }
            return result;
        }

        result.semantic_success = true;
    } catch (const std::exception& e) {
        result.errors.push_back(std::string("Semantic error: ") + e.what());
        return result;
    }

    return result;
}

// ============================================================================
// TEST CASES
// ============================================================================

void testSimpleProgram() {
    printTestHeader("TEST 1: Simple Valid Program");

    std::string code = R"(
        int main() {
            int x = 5;
            int y = 10;
            int sum = x + y;
            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testFunctionDeclaration() {
    printTestHeader("TEST 2: Function Declaration and Call");

    std::string code = R"(
        int add(int a, int b) {
            return a + b;
        }

        int main() {
            int result = add(5, 10);
            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testStructOperations() {
    printTestHeader("TEST 3: Struct Declaration and Usage");

    std::string code = R"(
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            p.x = 10;
            p.y = 20;
            int sum = p.x + p.y;
            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testControlFlow() {
    printTestHeader("TEST 4: Control Flow Statements");

    std::string code = R"(
        int factorial(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }

        int main() {
            int i = 0;
            int sum = 0;

            while (i < 10) {
                sum = sum + i;
                i = i + 1;
            }

            for (i = 0; i < 5; i = i + 1) {
                sum = sum + factorial(i);
            }

            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testPointerOperations() {
    printTestHeader("TEST 5: Pointer Operations");

    std::string code = R"(
        int main() {
            int x = 42;
            int *ptr = &x;
            int value = *ptr;

            int arr[10];
            int *p = arr;
            p = p + 5;

            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testTypeConversions() {
    printTestHeader("TEST 6: Implicit Type Conversions");

    std::string code = R"(
        int main() {
            char c = 'A';
            int i = c + 1;

            float f = 3.14;
            float result = i + f;

            int arr[5];
            int *ptr = arr;

            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testErrorDetection() {
    printTestHeader("TEST 7: Error Detection - Type Mismatch");

    std::string code = R"(
        int main() {
            int x = 5;
            x = "string";
            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);

    // This SHOULD fail semantic analysis
    bool correctlyFailed = !result.semantic_success && !result.errors.empty();
    printTestResult("Correctly detected type mismatch", correctlyFailed);

    if (!result.errors.empty()) {
        std::cout << "Expected errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testErrorDetection2() {
    printTestHeader("TEST 8: Error Detection - Undeclared Variable");

    std::string code = R"(
        int main() {
            int x = undeclared_var + 5;
            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);

    // This SHOULD fail semantic analysis
    bool correctlyFailed = !result.semantic_success && !result.errors.empty();
    printTestResult("Correctly detected undeclared variable", correctlyFailed);

    if (!result.errors.empty()) {
        std::cout << "Expected errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testErrorDetection3() {
    printTestHeader("TEST 9: Error Detection - Invalid Pointer Arithmetic");

    std::string code = R"(
        int main() {
            int *p1;
            int *p2;
            int *result = p1 * p2;
            return 0;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);

    // This SHOULD fail semantic analysis
    bool correctlyFailed = !result.semantic_success && !result.errors.empty();
    printTestResult("Correctly detected invalid pointer arithmetic", correctlyFailed);

    if (!result.errors.empty()) {
        std::cout << "Expected errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

void testComplexProgram() {
    printTestHeader("TEST 10: Complex Program - All Features");

    std::string code = R"(
        struct Node {
            int data;
            struct Node *next;
        };

        int globalCounter = 0;

        int add(int a, int b) {
            return a + b;
        }

        int factorial(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }

        int main() {
            int i = 0;
            int sum = 0;

            struct Node node;
            node.data = 42;

            for (i = 0; i < 5; i = i + 1) {
                sum = sum + factorial(i);
                globalCounter = globalCounter + 1;
            }

            while (sum > 0) {
                sum = sum - 1;
            }

            int result = add(sum, node.data);

            if (result > 100) {
                result = 100;
            } else {
                result = result + 1;
            }

            return result;
        }
    )";

    TestResult result = runFullPipeline(code);

    printTestResult("Lexer stage", result.lexer_success);
    printTestResult("Parser stage", result.parser_success);
    printTestResult("Semantic analysis stage", result.semantic_success);

    bool allPassed = result.lexer_success && result.parser_success && result.semantic_success;
    printTestResult("Complete pipeline", allPassed);

    if (!result.errors.empty()) {
        std::cout << "Errors:\n";
        for (const auto& error : result.errors) {
            std::cout << "  - " << error << "\n";
        }
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "INTEGRATION TEST SUITE\n";
    std::cout << "Testing: Lexer → Parser → Semantic Analyzer\n";
    std::cout << "========================================\n";

    int totalTests = 10;

    // Valid programs
    testSimpleProgram();
    testFunctionDeclaration();
    testStructOperations();
    testControlFlow();
    testPointerOperations();
    testTypeConversions();

    // Error detection tests
    testErrorDetection();
    testErrorDetection2();
    testErrorDetection3();

    // Complex program
    testComplexProgram();

    std::cout << "\n========================================\n";
    std::cout << "Integration Tests Complete!\n";
    std::cout << "Total Tests: " << totalTests << "\n";
    std::cout << "========================================\n";

    return 0;
}
