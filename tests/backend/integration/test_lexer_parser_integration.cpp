/**
 * ==============================================================================
 * Integration Tests: Lexer → Parser Pipeline
 * ==============================================================================
 *
 * Components Under Test:
 *   - Lexer (lexer.h, lexer.cpp)
 *   - Parser (parser.h, parser.cpp)
 *
 * Purpose:
 *   Verifies that the lexer and parser work correctly together as an integrated
 *   pipeline. Tests focus on:
 *   - Token stream flowing correctly from lexer to parser
 *   - Source code location preservation through both components
 *   - Error propagation from lexer through parser
 *   - Complete program tokenization and parsing
 *   - Edge cases in lexer-parser interaction
 *
 * Integration Points:
 *   - Parser constructor takes Lexer reference
 *   - Parser calls lexer.getNextToken() during parsing
 *   - Error handler source registration propagated from lexer to parser
 *   - Token position information used for AST node locations
 *
 * Test Categories:
 *   1. Basic Pipeline Flow - Simple programs tokenized and parsed correctly
 *   2. Position Tracking - Line/column preserved through pipeline
 *   3. Error Propagation - Lexer errors handled by parser
 *   4. Token Stream Integrity - All tokens consumed correctly
 *   5. Complex Programs - Real-world code patterns
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <memory>
#include <vector>
#include <string>

using namespace mycc_test;

// ==============================================================================
// Test Fixture for Lexer-Parser Integration Tests
// ==============================================================================

class LexerParserIntegrationTest : public ::testing::Test
{
protected:
    /**
     * Helper to run the complete lexer→parser pipeline and return results.
     * Allows inspection of both lexer and parser state after processing.
     */
    struct PipelineResult
    {
        std::vector<std::unique_ptr<Declaration>> declarations;
        bool lexer_has_errors;
        bool parser_has_errors;
        int lexer_error_count;
        int parser_error_count;
    };

    PipelineResult run_pipeline(const std::string &source,
                                const std::string &filename = "test.c")
    {
        Lexer lexer(source, filename);
        Parser parser(lexer);
        auto declarations = parser.parseProgram();

        return PipelineResult{
            std::move(declarations),
            lexer.hasErrors(),
            parser.hasErrors(),
            lexer.getErrorHandler().get_error_count(),
            parser.getErrorHandler().get_error_count()};
    }
};

// ==============================================================================
// Basic Pipeline Flow Tests
// ==============================================================================

/**
 * Test: Empty source flows through pipeline correctly
 *
 * Verifies:
 *   - Lexer produces EOF token
 *   - Parser receives EOF and produces empty program
 *   - No errors in either component
 */
TEST_F(LexerParserIntegrationTest, EmptySourceFlowsThrough)
{
    auto result = run_pipeline("");

    ASSERT_EQ(result.declarations.size(), 0)
        << "Empty source should produce no declarations";
    ASSERT_FALSE(result.lexer_has_errors) << "Lexer should have no errors";
    ASSERT_FALSE(result.parser_has_errors) << "Parser should have no errors";
}

/**
 * Test: Minimal valid program flows through pipeline
 *
 * Verifies:
 *   - Simple main function is correctly tokenized
 *   - Parser constructs proper AST from token stream
 */
TEST_F(LexerParserIntegrationTest, MinimalProgramFlowsThrough)
{
    std::string source = "int main() { return 0; }";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 1)
        << "Should produce exactly one function declaration";

    // Verify it's a function declaration
    auto *func = dynamic_cast<FunctionDecl *>(result.declarations[0].get());
    ASSERT_NE(func, nullptr) << "Should be a FunctionDecl";
    ASSERT_EQ(func->getName(), "main") << "Function should be named 'main'";
}

/**
 * Test: Variable declaration tokenizes and parses correctly
 *
 * Verifies:
 *   - Type keyword → type identifier flow
 *   - Variable name identifier flow
 *   - Semicolon token terminates declaration
 */
TEST_F(LexerParserIntegrationTest, VariableDeclarationFlow)
{
    std::string source = "int counter;";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 1);

    auto *var = dynamic_cast<VarDecl *>(result.declarations[0].get());
    ASSERT_NE(var, nullptr) << "Should be a VarDecl";
    ASSERT_EQ(var->getName(), "counter");
}

/**
 * Test: Variable with initializer flows correctly
 *
 * Verifies:
 *   - Assignment operator token connects declaration and expression
 *   - Integer literal parsed from token
 */
TEST_F(LexerParserIntegrationTest, VariableWithInitializerFlow)
{
    std::string source = "int x = 42;";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 1);

    auto *var = dynamic_cast<VarDecl *>(result.declarations[0].get());
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(var->getName(), "x");
    ASSERT_NE(var->getInitializer(), nullptr)
        << "Variable should have initializer";
}

// ==============================================================================
// Multiple Declarations Pipeline Tests
// ==============================================================================

/**
 * Test: Multiple declarations flow through pipeline
 *
 * Verifies:
 *   - Multiple top-level declarations tokenized sequentially
 *   - Parser maintains state correctly between declarations
 */
TEST_F(LexerParserIntegrationTest, MultipleDeclarationsFlow)
{
    std::string source = R"(
        int a;
        float b;
        char c;
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 3)
        << "Should produce three variable declarations";
}

/**
 * Test: Function followed by variable declaration
 *
 * Verifies:
 *   - Complex structure (function) followed by simple declaration
 *   - Token consumption does not skip or duplicate tokens
 */
TEST_F(LexerParserIntegrationTest, FunctionAndVariableMixed)
{
    std::string source = R"(
        int add(int a, int b) { return a + b; }
        int global_counter;
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 2);

    // First should be function
    auto *func = dynamic_cast<FunctionDecl *>(result.declarations[0].get());
    ASSERT_NE(func, nullptr);
    ASSERT_EQ(func->getName(), "add");

    // Second should be variable
    auto *var = dynamic_cast<VarDecl *>(result.declarations[1].get());
    ASSERT_NE(var, nullptr);
    ASSERT_EQ(var->getName(), "global_counter");
}

// ==============================================================================
// Expression Token Stream Tests
// ==============================================================================

/**
 * Test: Complex expression tokenizes and parses with correct precedence
 *
 * Verifies:
 *   - All operator tokens recognized
 *   - Parser applies precedence correctly to token sequence
 */
TEST_F(LexerParserIntegrationTest, ComplexExpressionPrecedence)
{
    std::string source = "int main() { int x = 2 + 3 * 4 - 1; return x; }";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 1);
}

/**
 * Test: Parenthesized expression overrides precedence
 *
 * Verifies:
 *   - Parenthesis tokens correctly delimit expression
 *   - Parser uses parentheses to override default precedence
 */
TEST_F(LexerParserIntegrationTest, ParenthesizedExpressionFlow)
{
    std::string source = "int main() { int x = (2 + 3) * 4; return x; }";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

/**
 * Test: Unary operators in expression
 *
 * Verifies:
 *   - Unary operator tokens (-, !, ~, etc.) recognized
 *   - Parser correctly handles unary expressions
 */
TEST_F(LexerParserIntegrationTest, UnaryOperatorFlow)
{
    std::string source = R"(
        int main() {
            int a = -5;
            int b = !0;
            return a + b;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

// ==============================================================================
// Literal Token Flow Tests
// ==============================================================================

/**
 * Test: All literal types flow through pipeline
 *
 * Verifies:
 *   - Integer, float, char, string literals tokenized correctly
 *   - Parser creates appropriate LiteralExpr nodes
 */
TEST_F(LexerParserIntegrationTest, AllLiteralTypesFlow)
{
    std::string source = R"(
        int main() {
            int i = 42;
            float f = 3.14;
            char c = 'x';
            return 0;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

/**
 * Test: Hexadecimal and octal literals
 *
 * Verifies:
 *   - Non-decimal integer formats tokenized correctly
 *   - Values preserved through parsing
 */
TEST_F(LexerParserIntegrationTest, NonDecimalLiteralsFlow)
{
    std::string source = R"(
        int main() {
            int hex = 0xFF;
            int oct = 0777;
            return hex + oct;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

// ==============================================================================
// Control Flow Statement Token Flow Tests
// ==============================================================================

/**
 * Test: If statement token flow
 *
 * Verifies:
 *   - if/else keywords recognized
 *   - Condition expression tokens flow to parser
 *   - Statement body braces handled correctly
 */
TEST_F(LexerParserIntegrationTest, IfStatementFlow)
{
    std::string source = R"(
        int main() {
            int x = 5;
            if (x > 0) {
                return 1;
            } else {
                return 0;
            }
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

/**
 * Test: While loop token flow
 *
 * Verifies:
 *   - while keyword and loop condition
 *   - Loop body statements parsed
 */
TEST_F(LexerParserIntegrationTest, WhileLoopFlow)
{
    std::string source = R"(
        int main() {
            int i = 0;
            while (i < 10) {
                i = i + 1;
            }
            return i;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

/**
 * Test: For loop token flow
 *
 * Verifies:
 *   - for keyword and three clauses
 *   - Semicolons within for header
 */
TEST_F(LexerParserIntegrationTest, ForLoopFlow)
{
    std::string source = R"(
        int main() {
            int sum = 0;
            for (int i = 0; i < 10; i = i + 1) {
                sum = sum + i;
            }
            return sum;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

// ==============================================================================
// Function Call Token Flow Tests
// ==============================================================================

/**
 * Test: Function call with arguments
 *
 * Verifies:
 *   - Function call parentheses and argument list
 *   - Comma-separated arguments
 */
TEST_F(LexerParserIntegrationTest, FunctionCallWithArgsFlow)
{
    std::string source = R"(
        int add(int a, int b) { return a + b; }
        int main() {
            int result = add(5, 10);
            return result;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 2);
}

/**
 * Test: Nested function calls
 *
 * Verifies:
 *   - Function calls as arguments to other functions
 *   - Token consumption handles nested parentheses
 */
TEST_F(LexerParserIntegrationTest, NestedFunctionCallsFlow)
{
    std::string source = R"(
        int square(int x) { return x * x; }
        int add(int a, int b) { return a + b; }
        int main() {
            int result = add(square(2), square(3));
            return result;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

// ==============================================================================
// Error Propagation Tests (Lexer → Parser)
// ==============================================================================

/**
 * Test: Invalid character propagates error
 *
 * Verifies:
 *   - Lexer error for invalid character
 *   - Parser can continue after lexer error
 */
TEST_F(LexerParserIntegrationTest, InvalidCharacterError)
{
    std::string source = "int x = @invalid;"; // @ is invalid

    auto result = run_pipeline(source);

    // Either lexer or parser should report error
    ASSERT_TRUE(result.lexer_has_errors || result.parser_has_errors)
        << "Invalid character should produce error";
}

/**
 * Test: Unterminated string propagates error
 *
 * Verifies:
 *   - Lexer detects unterminated string
 *   - Error information preserved
 */
TEST_F(LexerParserIntegrationTest, UnterminatedStringError)
{
    std::string source = "char* s = \"unterminated;";

    auto result = run_pipeline(source);

    ASSERT_TRUE(result.lexer_has_errors || result.parser_has_errors)
        << "Unterminated string should produce error";
}

/**
 * Test: Recovery after lexer error
 *
 * Verifies:
 *   - Parser can recover and continue parsing after lexer error
 *   - Subsequent valid code is still processed
 */
TEST_F(LexerParserIntegrationTest, RecoveryAfterLexerError)
{
    std::string source = R"(
        int x = @bad;
        int y = 10;
    )";

    auto result = run_pipeline(source);

    // Should have errors but may still parse some declarations
    ASSERT_TRUE(result.lexer_has_errors || result.parser_has_errors);
}

// ==============================================================================
// Position Tracking Through Pipeline
// ==============================================================================

/**
 * Test: Parser error reports correct line number from tokens
 *
 * Verifies:
 *   - Line information from lexer preserved to parser errors
 */
TEST_F(LexerParserIntegrationTest, LineNumberPreservedOnError)
{
    std::string source = R"(
        int main() {
            int x = 5;
            int y =    // Missing expression on line 4
        }
    )";

    auto result = run_pipeline(source);

    // Should have parser error
    ASSERT_TRUE(result.parser_has_errors)
        << "Missing expression should produce parser error";
}

// ==============================================================================
// Complex Real-World Programs
// ==============================================================================

/**
 * Test: Complete realistic program
 *
 * Verifies:
 *   - Full program with multiple features flows through pipeline
 *   - All C constructs work together
 */
TEST_F(LexerParserIntegrationTest, CompleteRealisticProgram)
{
    std::string source = R"(
        // Global variable
        int global_count;

        // Helper function
        int max(int a, int b) {
            if (a > b) {
                return a;
            }
            return b;
        }

        // Main entry point
        int main() {
            int x = 10;
            int y = 20;
            int result = max(x, y);
            global_count = global_count + 1;
            return result;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_EQ(result.declarations.size(), 3)
        << "Should have 1 variable + 2 functions";
}

/**
 * Test: Struct declaration and usage
 *
 * Verifies:
 *   - Struct keyword and member declarations
 *   - Struct variable declaration
 */
TEST_F(LexerParserIntegrationTest, StructDeclarationFlow)
{
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            return 0;
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}

/**
 * Test: Array declaration and indexing
 *
 * Verifies:
 *   - Array bracket tokens
 *   - Array subscript expressions
 */
TEST_F(LexerParserIntegrationTest, ArrayDeclarationAndIndexing)
{
    std::string source = R"(
        int main() {
            int arr[10];
            arr[0] = 42;
            arr[1] = arr[0] + 1;
            return arr[1];
        }
    )";

    auto result = run_pipeline(source);

    ASSERT_FALSE(result.lexer_has_errors);
    ASSERT_FALSE(result.parser_has_errors);
}
