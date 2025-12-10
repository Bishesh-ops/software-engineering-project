/**
 * ==============================================================================
 * Integration Tests: Complete Frontend Pipeline
 * ==============================================================================
 *
 * Components Under Test:
 *   - Lexer (lexer.h, lexer.cpp)
 *   - Parser (parser.h, parser.cpp)
 *   - SemanticAnalyzer (semantic_analyzer.h, semantic_analyzer.cpp)
 *   - ErrorHandler (error_handler.h, error_handler.cpp)
 *
 * Purpose:
 *   End-to-end testing of the complete frontend compilation pipeline from
 *   source code to semantically analyzed AST. Tests focus on:
 *   - Complete pipeline execution for valid programs
 *   - Pipeline behavior with errors at different stages
 *   - Error message quality and location accuracy
 *   - Recovery capabilities throughout the pipeline
 *   - Real-world C program scenarios
 *
 * Integration Points:
 *   - Source code → Lexer → Token stream
 *   - Token stream → Parser → AST
 *   - AST → SemanticAnalyzer → Analyzed AST + Symbol Table
 *   - ErrorHandler aggregates errors from all stages
 *
 * Test Categories:
 *   1. Complete Valid Programs - End-to-end successful compilation
 *   2. Stage-Specific Errors - Errors originating at different stages
 *   3. Error Recovery - Pipeline continues after errors
 *   4. Sample Programs - Real-world code patterns
 *   5. Edge Cases - Unusual but valid constructs
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <memory>
#include <vector>
#include <string>
#include <sstream>

using namespace mycc_test;

// ==============================================================================
// Test Fixture for Complete Frontend Pipeline Tests
// ==============================================================================

class FrontendPipelineTest : public ::testing::Test
{
protected:
    /**
     * Comprehensive pipeline result with all stage information
     */
    struct FrontendResult
    {
        // Stage outputs
        std::vector<std::unique_ptr<Declaration>> declarations;

        // Error tracking per stage
        bool lexer_success;
        bool parser_success;
        bool semantic_success;
        bool overall_success;

        // Error counts
        int total_errors;
        int total_warnings;

        // Error messages for verification
        std::vector<std::string> error_messages;
    };

    /**
     * Execute the complete frontend pipeline
     */
    FrontendResult run_frontend(const std::string &source,
                                const std::string &filename = "test.c",
                                bool enable_warnings = false)
    {
        FrontendResult result;

        // Stage 1: Lexer
        Lexer lexer(source, filename);

        // Stage 2: Parser
        Parser parser(lexer);
        result.declarations = parser.parseProgram();

        result.lexer_success = !lexer.hasErrors();
        result.parser_success = !parser.hasErrors();

        // Stage 3: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.set_warnings_enabled(enable_warnings);
        analyzer.analyze_program(result.declarations);

        result.semantic_success = !analyzer.has_errors();

        // Aggregate results
        result.overall_success = result.lexer_success &&
                                 result.parser_success &&
                                 result.semantic_success;

        result.total_errors = lexer.getErrorHandler().get_error_count() +
                              parser.getErrorHandler().get_error_count() +
                              analyzer.getErrorHandler().get_error_count();

        result.total_warnings = analyzer.getErrorHandler().get_warning_count();

        return result;
    }
};

// ==============================================================================
// Complete Valid Program Tests
// ==============================================================================

/**
 * Test: Minimal valid program passes all stages
 */
TEST_F(FrontendPipelineTest, MinimalProgramPassesAllStages)
{
    std::string source = "int main() { return 0; }";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success)
        << "Minimal valid program should pass all frontend stages";
    ASSERT_TRUE(result.lexer_success);
    ASSERT_TRUE(result.parser_success);
    ASSERT_TRUE(result.semantic_success);
    ASSERT_EQ(result.total_errors, 0);
}

/**
 * Test: Program with all basic features
 */
TEST_F(FrontendPipelineTest, ComprehensiveProgramPassesAllStages)
{
    std::string source = R"(
        // Global variable
        int global_count;

        // Function with parameters
        int add(int a, int b) {
            return a + b;
        }

        // Function with control flow
        int max(int x, int y) {
            if (x > y) {
                return x;
            }
            return y;
        }

        // Main entry point
        int main() {
            int result = 0;
            result = add(5, 10);
            result = max(result, 20);
            global_count = result;
            return 0;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success)
        << "Comprehensive valid program should pass all stages";
    ASSERT_EQ(result.total_errors, 0);
}

/**
 * Test: Program with loops
 */
TEST_F(FrontendPipelineTest, LoopConstructsPass)
{
    std::string source = R"(
        int main() {
            int sum = 0;

            // While loop
            int i = 0;
            while (i < 5) {
                sum = sum + i;
                i = i + 1;
            }

            // For loop
            for (int j = 0; j < 5; j = j + 1) {
                sum = sum + j;
            }

            return sum;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

/**
 * Test: Program with arrays
 */
TEST_F(FrontendPipelineTest, ArrayOperationsPass)
{
    std::string source = R"(
        int main() {
            int arr[10];
            arr[0] = 42;
            arr[1] = arr[0] + 1;
            return arr[1];
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

// ==============================================================================
// Stage-Specific Error Tests
// ==============================================================================

/**
 * Test: Lexer error stops early
 */
TEST_F(FrontendPipelineTest, LexerErrorDetected)
{
    std::string source = "int x = @invalid;"; // @ is not valid

    auto result = run_frontend(source);

    ASSERT_FALSE(result.overall_success)
        << "Invalid token should cause failure";
    ASSERT_FALSE(result.lexer_success)
        << "Error should originate in lexer";
}

/**
 * Test: Parser error with valid tokens
 */
TEST_F(FrontendPipelineTest, ParserSyntaxError)
{
    std::string source = "int int int;"; // Invalid syntax

    auto result = run_frontend(source);

    ASSERT_FALSE(result.overall_success);
    ASSERT_FALSE(result.parser_success)
        << "Invalid syntax should cause parser error";
}

/**
 * Test: Semantic error with valid syntax
 */
TEST_F(FrontendPipelineTest, SemanticErrorWithValidSyntax)
{
    std::string source = R"(
        int main() {
            int x = undefined_var;  // Semantically invalid
            return x;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_FALSE(result.overall_success);
    ASSERT_TRUE(result.lexer_success) << "Lexer should succeed";
    ASSERT_TRUE(result.parser_success) << "Parser should succeed";
    ASSERT_FALSE(result.semantic_success) << "Semantic analysis should fail";
}

/**
 * Test: Missing semicolon (parser error)
 */
TEST_F(FrontendPipelineTest, MissingSemicolonError)
{
    std::string source = R"(
        int main() {
            int x = 5  // Missing semicolon
            return x;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_FALSE(result.overall_success);
    ASSERT_FALSE(result.parser_success);
}

/**
 * Test: Unclosed brace (parser error)
 */
TEST_F(FrontendPipelineTest, UnclosedBraceError)
{
    std::string source = R"(
        int main() {
            int x = 5;
        // Missing closing brace
    )";

    auto result = run_frontend(source);

    ASSERT_FALSE(result.overall_success);
    ASSERT_FALSE(result.parser_success);
}

// ==============================================================================
// Error Recovery Tests
// ==============================================================================

/**
 * Test: Pipeline continues after first error
 */
TEST_F(FrontendPipelineTest, ContinuesAfterFirstError)
{
    std::string source = R"(
        int main() {
            int x = undefined1;  // Error 1
            int y = undefined2;  // Error 2
            return 0;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_FALSE(result.overall_success);
    ASSERT_GE(result.total_errors, 2)
        << "Should detect multiple errors, not stop at first";
}

// ==============================================================================
// Sample Real-World Programs
// ==============================================================================

/**
 * Test: Fibonacci function
 */
TEST_F(FrontendPipelineTest, FibonacciProgram)
{
    std::string source = R"(
        int fibonacci(int n) {
            if (n <= 1) {
                return n;
            }
            return fibonacci(n - 1) + fibonacci(n - 2);
        }

        int main() {
            int result = fibonacci(10);
            return result;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success)
        << "Fibonacci program should compile successfully";
}

/**
 * Test: Simple calculator operations
 */
TEST_F(FrontendPipelineTest, CalculatorProgram)
{
    std::string source = R"(
        int add(int a, int b) { return a + b; }
        int sub(int a, int b) { return a - b; }
        int mul(int a, int b) { return a * b; }
        int div(int a, int b) { return a / b; }

        int main() {
            int result = add(10, 5);
            result = sub(result, 3);
            result = mul(result, 2);
            result = div(result, 4);
            return result;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

/**
 * Test: Nested control structures
 */
TEST_F(FrontendPipelineTest, NestedControlStructures)
{
    std::string source = R"(
        int main() {
            int sum = 0;
            for (int i = 0; i < 5; i = i + 1) {
                for (int j = 0; j < 5; j = j + 1) {
                    if (i == j) {
                        sum = sum + 1;
                    } else {
                        sum = sum + 2;
                    }
                }
            }
            return sum;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

// ==============================================================================
// Edge Case Tests
// ==============================================================================

/**
 * Test: Empty program
 */
TEST_F(FrontendPipelineTest, EmptyProgram)
{
    std::string source = "";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success)
        << "Empty program should be valid";
    ASSERT_EQ(result.declarations.size(), 0);
}

/**
 * Test: Comments only
 */
TEST_F(FrontendPipelineTest, CommentsOnlyProgram)
{
    std::string source = R"(
        // This is a comment
        /* This is a
           multi-line comment */
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
    ASSERT_EQ(result.declarations.size(), 0);
}

/**
 * Test: Deeply nested blocks
 */
TEST_F(FrontendPipelineTest, DeeplyNestedBlocks)
{
    std::string source = R"(
        int main() {
            int a = 1;
            {
                int b = 2;
                {
                    int c = 3;
                    {
                        int d = 4;
                        {
                            int e = a + b + c + d;
                            return e;
                        }
                    }
                }
            }
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

/**
 * Test: Many parameters in function
 */
TEST_F(FrontendPipelineTest, ManyFunctionParameters)
{
    std::string source = R"(
        int sum_all(int a, int b, int c, int d, int e) {
            return a + b + c + d + e;
        }

        int main() {
            return sum_all(1, 2, 3, 4, 5);
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

/**
 * Test: Long expression chain
 */
TEST_F(FrontendPipelineTest, LongExpressionChain)
{
    std::string source = R"(
        int main() {
            int result = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
            result = result * 2 - 10 / 5 + 3 - 1 + 4 * 2;
            return result;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

// ==============================================================================
// Warning Tests
// ==============================================================================

/**
 * Test: Warnings detected but program valid
 */
TEST_F(FrontendPipelineTest, WarningsWithValidProgram)
{
    std::string source = R"(
        int main() {
            int unused_var = 42;  // Should trigger unused variable warning
            return 0;
        }
    )";

    auto result = run_frontend(source, "test.c", true /* enable warnings */);

    ASSERT_TRUE(result.overall_success)
        << "Warnings should not prevent success";
    ASSERT_GT(result.total_warnings, 0)
        << "Should have at least one warning";
}

/**
 * Test: Multiple warnings accumulated
 */
TEST_F(FrontendPipelineTest, MultipleWarningsAccumulated)
{
    std::string source = R"(
        int main() {
            int unused1 = 1;
            int unused2 = 2;
            int unused3 = 3;
            return 0;
        }
    )";

    auto result = run_frontend(source, "test.c", true);

    ASSERT_TRUE(result.overall_success);
    ASSERT_GE(result.total_warnings, 3)
        << "Should detect multiple unused variable warnings";
}

// ==============================================================================
// Struct and Complex Type Tests
// ==============================================================================

/**
 * Test: Struct with functions
 */
TEST_F(FrontendPipelineTest, StructWithFunctions)
{
    std::string source = R"(
        struct Rectangle {
            int width;
            int height;
        };

        int main() {
            struct Rectangle r;
            return 0;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}

/**
 * Test: Multiple structs
 */
TEST_F(FrontendPipelineTest, MultipleStructs)
{
    std::string source = R"(
        struct Point { int x; int y; };
        struct Rectangle { int x; int y; int w; int h; };

        int main() {
            struct Point p;
            struct Rectangle r;
            return 0;
        }
    )";

    auto result = run_frontend(source);

    ASSERT_TRUE(result.overall_success);
}
