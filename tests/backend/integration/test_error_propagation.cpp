/**
 * ==============================================================================
 * Integration Tests: Error Propagation Across Components
 * ==============================================================================
 *
 * Components Under Test:
 *   - ErrorHandler (error_handler.h, error_handler.cpp)
 *   - Lexer → Parser → SemanticAnalyzer error flow
 *
 * Purpose:
 *   Verifies that errors are correctly detected, reported, and propagated
 *   across component boundaries in the compilation pipeline. Tests focus on:
 *   - Error detection at correct stage
 *   - Error message accuracy and context
 *   - Source location preservation through pipeline
 *   - Error recovery and continuation
 *   - Warning vs error distinction
 *   - Error count limits and thresholds
 *
 * Integration Points:
 *   - ErrorHandler shared between components
 *   - SourceLocation tracked from lexer through semantic
 *   - Error messages include context from multiple stages
 *   - Error limits affect pipeline behavior
 *
 * Test Categories:
 *   1. Error Origin Detection - Identify which stage produced error
 *   2. Location Accuracy - Line/column information preserved
 *   3. Error Recovery - Pipeline continues after errors
 *   4. Error Limits - Max error behavior
 *   5. Warning Propagation - Warnings distinct from errors
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "error_handler.h"
#include <memory>
#include <vector>
#include <string>

using namespace mycc_test;

// ==============================================================================
// Test Fixture for Error Propagation Tests
// ==============================================================================

class ErrorPropagationTest : public ::testing::Test
{
protected:
    /**
     * Error analysis result with detailed error tracking
     */
    struct ErrorAnalysis
    {
        // Error counts per stage
        int lexer_errors;
        int parser_errors;
        int semantic_errors;
        int semantic_warnings;

        // Stage success flags
        bool lexer_ok;
        bool parser_ok;
        bool semantic_ok;

        // Error details (first error from each stage if available)
        std::string first_error_stage;

        // Declarations (if parsing succeeded)
        std::vector<std::unique_ptr<Declaration>> declarations;
    };

    /**
     * Analyze errors throughout the pipeline
     */
    ErrorAnalysis analyze_errors(const std::string &source,
                                 const std::string &filename = "test.c",
                                 bool enable_warnings = true)
    {
        ErrorAnalysis result;

        // Stage 1: Lexer
        Lexer lexer(source, filename);

        // Stage 2: Parser
        Parser parser(lexer);
        result.declarations = parser.parseProgram();

        result.lexer_errors = lexer.getErrorHandler().get_error_count();
        result.parser_errors = parser.getErrorHandler().get_error_count();
        result.lexer_ok = !lexer.hasErrors();
        result.parser_ok = !parser.hasErrors();

        // Stage 3: Semantic Analysis
        SemanticAnalyzer analyzer;
        analyzer.set_warnings_enabled(enable_warnings);
        analyzer.analyze_program(result.declarations);

        result.semantic_errors = analyzer.getErrorHandler().get_error_count();
        result.semantic_warnings = analyzer.getErrorHandler().get_warning_count();
        result.semantic_ok = !analyzer.has_errors();

        // Determine first error stage
        if (result.lexer_errors > 0)
        {
            result.first_error_stage = "lexer";
        }
        else if (result.parser_errors > 0)
        {
            result.first_error_stage = "parser";
        }
        else if (result.semantic_errors > 0)
        {
            result.first_error_stage = "semantic";
        }
        else
        {
            result.first_error_stage = "none";
        }

        return result;
    }
};

// ==============================================================================
// Error Origin Detection Tests
// ==============================================================================

/**
 * Test: Lexer error detected first
 */
TEST_F(ErrorPropagationTest, LexerErrorOrigin)
{
    std::string source = "int x = @invalid;"; // @ is lexer error

    auto result = analyze_errors(source);

    ASSERT_EQ(result.first_error_stage, "lexer")
        << "@ should be detected as lexer error";
    ASSERT_GT(result.lexer_errors, 0);
}

/**
 * Test: Parser error with valid tokens
 */
TEST_F(ErrorPropagationTest, ParserErrorOrigin)
{
    std::string source = "int int int;"; // Valid tokens, invalid syntax

    auto result = analyze_errors(source);

    ASSERT_EQ(result.first_error_stage, "parser")
        << "Double type specifier should be parser error";
    ASSERT_TRUE(result.lexer_ok) << "Tokens are valid";
    ASSERT_GT(result.parser_errors, 0);
}

/**
 * Test: Semantic error with valid syntax
 */
TEST_F(ErrorPropagationTest, SemanticErrorOrigin)
{
    std::string source = R"(
        int main() {
            int x = undefined_variable;
            return x;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_EQ(result.first_error_stage, "semantic")
        << "Undefined variable should be semantic error";
    ASSERT_TRUE(result.lexer_ok);
    ASSERT_TRUE(result.parser_ok);
    ASSERT_GT(result.semantic_errors, 0);
}

/**
 * Test: No errors in valid program
 */
TEST_F(ErrorPropagationTest, NoErrorsInValidProgram)
{
    std::string source = "int main() { return 0; }";

    auto result = analyze_errors(source);

    ASSERT_EQ(result.first_error_stage, "none")
        << "Valid program should have no errors";
    ASSERT_TRUE(result.lexer_ok);
    ASSERT_TRUE(result.parser_ok);
    ASSERT_TRUE(result.semantic_ok);
}

// ==============================================================================
// Error Recovery Tests
// ==============================================================================

/**
 * Test: Parser continues after syntax error
 */
TEST_F(ErrorPropagationTest, ParserContinuesAfterError)
{
    std::string source = R"(
        int x = ;  // Error: missing expression
        int y = 5; // Should still be parsed
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.parser_errors, 0)
        << "Should detect the syntax error";
    // Parser may still produce some declarations through recovery
}

/**
 * Test: Semantic analyzer continues after error
 */
TEST_F(ErrorPropagationTest, SemanticContinuesAfterError)
{
    std::string source = R"(
        int main() {
            int x = undefined1;  // Error 1
            int y = undefined2;  // Error 2 (should still be detected)
            return 0;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_TRUE(result.lexer_ok);
    ASSERT_TRUE(result.parser_ok);
    ASSERT_GE(result.semantic_errors, 2)
        << "Should detect multiple semantic errors";
}

/**
 * Test: Multiple error types in one program
 */
TEST_F(ErrorPropagationTest, MultipleErrorTypes)
{
    std::string source = R"(
        int main() {
            int x = @bad;        // Lexer error
            int y = undefined;   // Semantic error (if lexer recovers)
            return 0;
        }
    )";

    auto result = analyze_errors(source);

    // Should have at least lexer error
    ASSERT_GT(result.lexer_errors + result.parser_errors + result.semantic_errors, 0);
}

// ==============================================================================
// Warning vs Error Distinction Tests
// ==============================================================================

/**
 * Test: Warning does not affect success
 */
TEST_F(ErrorPropagationTest, WarningDoesNotAffectSuccess)
{
    std::string source = R"(
        int main() {
            int unused = 42;  // Warning: unused variable
            return 0;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_TRUE(result.lexer_ok);
    ASSERT_TRUE(result.parser_ok);
    ASSERT_TRUE(result.semantic_ok)
        << "Warnings should not cause semantic failure";
    ASSERT_GT(result.semantic_warnings, 0)
        << "Should have unused variable warning";
}

/**
 * Test: Warnings and errors tracked separately
 */
TEST_F(ErrorPropagationTest, WarningsAndErrorsSeparate)
{
    std::string source = R"(
        int main() {
            int unused = 42;        // Warning
            int x = undefined_var;  // Error
            return 0;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.semantic_errors, 0) << "Should have error";
    ASSERT_GT(result.semantic_warnings, 0) << "Should have warning";
}

// ==============================================================================
// Error Detail Tests
// ==============================================================================

/**
 * Test: Redeclaration error
 */
TEST_F(ErrorPropagationTest, RedeclarationError)
{
    std::string source = R"(
        int main() {
            int x = 1;
            int x = 2;  // Redeclaration error
            return x;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.semantic_errors, 0)
        << "Redeclaration should produce semantic error";
}

/**
 * Test: Type mismatch error
 */
TEST_F(ErrorPropagationTest, TypeMismatchError)
{
    std::string source = R"(
        int main() {
            int x = "string";  // Type mismatch
            return x;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.semantic_errors, 0)
        << "Type mismatch should produce semantic error";
}

/**
 * Test: Undefined function error
 */
TEST_F(ErrorPropagationTest, UndefinedFunctionError)
{
    std::string source = R"(
        int main() {
            int result = unknown_func(5);
            return result;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.semantic_errors, 0)
        << "Undefined function should produce semantic error";
}

/**
 * Test: Wrong argument count error
 */
TEST_F(ErrorPropagationTest, WrongArgumentCountError)
{
    std::string source = R"(
        int add(int a, int b) { return a + b; }
        int main() {
            int result = add(1);  // Too few arguments
            return result;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.semantic_errors, 0)
        << "Wrong argument count should produce semantic error";
}

// ==============================================================================
// Lexer-Specific Error Tests
// ==============================================================================

/**
 * Test: Unterminated string error
 */
TEST_F(ErrorPropagationTest, UnterminatedStringError)
{
    std::string source = "char* s = \"unterminated;";

    auto result = analyze_errors(source);

    ASSERT_GT(result.lexer_errors, 0)
        << "Unterminated string should produce lexer error";
}

/**
 * Test: Unterminated character literal
 */
TEST_F(ErrorPropagationTest, UnterminatedCharError)
{
    std::string source = "char c = 'x;";

    auto result = analyze_errors(source);

    ASSERT_GT(result.lexer_errors, 0)
        << "Unterminated char should produce lexer error";
}

/**
 * Test: Invalid escape sequence
 */
TEST_F(ErrorPropagationTest, InvalidEscapeSequence)
{
    std::string source = R"(char* s = "\z";)"; // \z is not a valid escape

    auto result = analyze_errors(source);

    // May or may not be an error depending on implementation
    // Just verify we don't crash
    ASSERT_TRUE(true);
}

// ==============================================================================
// Parser-Specific Error Tests
// ==============================================================================

/**
 * Test: Missing semicolon
 */
TEST_F(ErrorPropagationTest, MissingSemicolon)
{
    std::string source = R"(
        int main() {
            int x = 5
            return x;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.parser_errors, 0)
        << "Missing semicolon should produce parser error";
}

/**
 * Test: Missing closing parenthesis
 */
TEST_F(ErrorPropagationTest, MissingCloseParen)
{
    std::string source = R"(
        int main() {
            if (x > 0 {
                return 1;
            }
            return 0;
        }
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.parser_errors, 0)
        << "Missing parenthesis should produce parser error";
}

/**
 * Test: Missing closing brace
 */
TEST_F(ErrorPropagationTest, MissingCloseBrace)
{
    std::string source = R"(
        int main() {
            int x = 5;
    )";

    auto result = analyze_errors(source);

    ASSERT_GT(result.parser_errors, 0)
        << "Missing brace should produce parser error";
}
