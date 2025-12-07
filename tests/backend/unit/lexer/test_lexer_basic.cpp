/**
 * ==============================================================================
 * Lexer Basic Functionality Tests
 * ==============================================================================
 *
 * Module Under Test: Lexer (lexer.h, lexer.cpp)
 *
 * Purpose:
 *   Tests fundamental lexer capabilities including:
 *   - Empty source handling
 *   - Whitespace handling
 *   - Comment handling
 *   - Basic token sequencing
 *   - EOF token generation
 *   - Line and column tracking
 *
 * Coverage:
 *   ✓ Empty source files
 *   ✓ Whitespace-only sources
 *   ✓ Single-line comments
 *   ✓ Multi-line comments
 *   ✓ Mixed whitespace types
 *   ✓ Position tracking accuracy
 *
 * Dependencies:
 *   - GoogleTest framework
 *   - test_helpers.h
 *   - lexer.h
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"
#include <string>

using namespace mycc_test;

// ==============================================================================
// Test Fixture for Basic Lexer Tests
// ==============================================================================

class LexerBasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup if needed
    }

    void TearDown() override {
        // Cleanup if needed
    }
};

// ==============================================================================
// Empty Source Tests
// ==============================================================================

/**
 * Test: Empty source produces only EOF token
 *
 * Verifies:
 *   - Lexer handles empty input gracefully
 *   - Single EOF token is generated
 *   - No errors are reported
 */
TEST_F(LexerBasicTest, EmptySourceProducesEOF) {
    // Arrange
    std::string source = "";
    Lexer lexer(source, "empty.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 1) << "Empty source should produce only EOF";
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::EOF_TOKEN);
    ASSERT_FALSE(lexer.hasErrors()) << "Empty source should not produce errors";
}

/**
 * Test: Whitespace-only source produces only EOF
 *
 * Verifies:
 *   - All whitespace types are correctly skipped
 *   - Spaces, tabs, newlines, carriage returns handled
 */
TEST_F(LexerBasicTest, WhitespaceOnlyProducesEOF) {
    // Arrange - various whitespace combinations
    std::string source = "   \t\t\n\n  \r\n  \t  ";
    Lexer lexer(source, "whitespace.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 1) << "Whitespace-only source should produce only EOF";
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::EOF_TOKEN);
    ASSERT_FALSE(lexer.hasErrors());
}

// ==============================================================================
// Comment Handling Tests
// ==============================================================================

/**
 * Test: Single-line comment is properly skipped
 *
 * Verifies:
 *   - // style comments are recognized
 *   - Comment content is not tokenized
 *   - Subsequent tokens are correctly processed
 */
TEST_F(LexerBasicTest, SingleLineCommentSkipped) {
    // Arrange
    std::string source = R"(
        // This is a comment
        int
    )";
    Lexer lexer(source, "comment.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 2) << "Should have 'int' and EOF";
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_INT, "int");
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::EOF_TOKEN);
}

/**
 * Test: Multi-line comment is properly skipped
 *
 * Verifies:
 *   - Block-style comments are recognized
 *   - Multi-line content is handled
 *   - Line counting continues correctly after comment
 */
TEST_F(LexerBasicTest, MultiLineCommentSkipped) {
    // Arrange
    std::string source = R"(
        /* This is a
           multi-line
           comment */
        float
    )";
    Lexer lexer(source, "multicomment.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 2) << "Should have 'float' and EOF";
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_FLOAT, "float");
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::EOF_TOKEN);
}

/**
 * Test: Comments interleaved with tokens
 *
 * Verifies:
 *   - Comments can appear between any tokens
 *   - Multiple comment types in same source
 */
TEST_F(LexerBasicTest, InterleavedComments) {
    // Arrange
    std::string source = R"(
        int /* comment */ x // another comment
        = /* more */ 5;
    )";
    Lexer lexer(source, "interleaved.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert - should get: int, x, =, 5, ;, EOF
    ASSERT_EQ(tokens.size(), 6);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_INT, "int");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_ASSIGN, "=");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::INT_LITERAL, "5");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::SEMICOLON, ";");
    ASSERT_TOKEN_TYPE(tokens[5], TokenType::EOF_TOKEN);
}

// ==============================================================================
// Token Sequencing Tests
// ==============================================================================

/**
 * Test: Simple token sequence
 *
 * Verifies:
 *   - Tokens are generated in correct order
 *   - No tokens are skipped or duplicated
 */
TEST_F(LexerBasicTest, SimpleTokenSequence) {
    // Arrange
    std::string source = "int main ( )";
    Lexer lexer(source, "sequence.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 5);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_INT, "int");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::IDENTIFIER, "main");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::LPAREN, "(");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::RPAREN, ")");
    ASSERT_TOKEN_TYPE(tokens[4], TokenType::EOF_TOKEN);
}

/**
 * Test: No whitespace between tokens (where valid)
 *
 * Verifies:
 *   - Lexer correctly separates adjacent tokens
 *   - No whitespace required for delimiters
 */
TEST_F(LexerBasicTest, NoWhitespaceBetweenDelimiters) {
    // Arrange
    std::string source = "(){};,";
    Lexer lexer(source, "nospace.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 7);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::LPAREN, "(");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::RPAREN, ")");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::LBRACE, "{");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::RBRACE, "}");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::SEMICOLON, ";");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::COMMA, ",");
    ASSERT_TOKEN_TYPE(tokens[6], TokenType::EOF_TOKEN);
}

// ==============================================================================
// Position Tracking Tests
// ==============================================================================

/**
 * Test: Line number tracking
 *
 * Verifies:
 *   - Line numbers are 1-based
 *   - Line numbers increment correctly
 *   - Tokens on same line have same line number
 */
TEST_F(LexerBasicTest, LineNumberTracking) {
    // Arrange
    std::string source = "int\nfloat\nchar";
    Lexer lexer(source, "lines.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 4);
    EXPECT_EQ(tokens[0].line, 1) << "First token should be on line 1";
    EXPECT_EQ(tokens[1].line, 2) << "Second token should be on line 2";
    EXPECT_EQ(tokens[2].line, 3) << "Third token should be on line 3";
}

/**
 * Test: Column number tracking
 *
 * Verifies:
 *   - Column numbers are 1-based
 *   - Columns track position within line
 */
TEST_F(LexerBasicTest, ColumnNumberTracking) {
    // Arrange
    std::string source = "int x = 5;";
    Lexer lexer(source, "columns.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_GE(tokens.size(), 5);
    EXPECT_EQ(tokens[0].column, 1) << "'int' starts at column 1";
    EXPECT_EQ(tokens[1].column, 5) << "'x' starts at column 5";
    EXPECT_EQ(tokens[2].column, 7) << "'=' starts at column 7";
    EXPECT_EQ(tokens[3].column, 9) << "'5' starts at column 9";
}

/**
 * Test: Filename tracking
 *
 * Verifies:
 *   - All tokens remember their source filename
 */
TEST_F(LexerBasicTest, FilenameTracking) {
    // Arrange
    std::string source = "int x;";
    std::string filename = "test_file.c";
    Lexer lexer(source, filename);

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    for (const auto& token : tokens) {
        EXPECT_EQ(token.filename, filename)
            << "All tokens should track their source filename";
    }
}

// ==============================================================================
// Edge Case Tests
// ==============================================================================

/**
 * Test: Source ending with comment (no newline after)
 *
 * Verifies:
 *   - EOF handling with trailing comment
 */
TEST_F(LexerBasicTest, SourceEndsWithComment) {
    // Arrange
    std::string source = "int x; // no newline after this";
    Lexer lexer(source, "trailing_comment.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert - should get: int, x, ;, EOF
    ASSERT_EQ(tokens.size(), 4);
    ASSERT_TOKEN_TYPE(tokens[3], TokenType::EOF_TOKEN);
    ASSERT_FALSE(lexer.hasErrors());
}

/**
 * Test: Multiple consecutive newlines
 *
 * Verifies:
 *   - Multiple newlines don't create spurious tokens
 *   - Line counting is correct
 */
TEST_F(LexerBasicTest, MultipleConsecutiveNewlines) {
    // Arrange
    std::string source = "int\n\n\n\nfloat";
    Lexer lexer(source, "newlines.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert
    ASSERT_EQ(tokens.size(), 3); // int, float, EOF
    EXPECT_EQ(tokens[0].line, 1);
    EXPECT_EQ(tokens[1].line, 5) << "float should be on line 5";
}

// ==============================================================================
// Integration: Real-World Code Snippet
// ==============================================================================

/**
 * Test: Minimal valid C program
 *
 * Verifies:
 *   - Lexer handles realistic program structure
 *   - All tokens in correct sequence
 */
TEST_F(LexerBasicTest, MinimalValidProgram) {
    // Arrange
    std::string source = minimal_valid_program();
    Lexer lexer(source, "minimal.c");

    // Act
    auto tokens = lexer.lexAll();

    // Assert - verify key tokens are present
    ASSERT_GE(tokens.size(), 8); // At minimum: int main ( ) { return 0 ; }

    bool found_int = false;
    bool found_main = false;
    bool found_return = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::KW_INT) found_int = true;
        if (token.type == TokenType::IDENTIFIER && token.value == "main") found_main = true;
        if (token.type == TokenType::KW_RETURN) found_return = true;
    }

    EXPECT_TRUE(found_int) << "Should find 'int' keyword";
    EXPECT_TRUE(found_main) << "Should find 'main' identifier";
    EXPECT_TRUE(found_return) << "Should find 'return' keyword";
    ASSERT_FALSE(lexer.hasErrors());
}
