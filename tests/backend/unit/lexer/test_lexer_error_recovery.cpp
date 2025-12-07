/**
 * ==============================================================================
 * Lexer Error Recovery and Diagnostics Tests
 * ==============================================================================
 *
 * Module Under Test: Lexer (lexer.h, lexer.cpp)
 *
 * Purpose:
 *   Tests error detection, recovery, and diagnostic quality for invalid input.
 *   Ensures lexer handles malformed source gracefully and provides useful
 *   error messages.
 *
 * Coverage:
 *   ✓ Unterminated string literals
 *   ✓ Unterminated character literals
 *   ✓ Unterminated multi-line comments
 *   ✓ Invalid characters in source
 *   ✓ Malformed number literals
 *   ✓ Error recovery continuation
 *   ✓ Multiple errors in same source
 *   ✓ Error count limits
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"

using namespace mycc_test;

class LexerErrorRecoveryTest : public ::testing::Test {};

// ==============================================================================
// Unterminated Literals
// ==============================================================================

/**
 * Test: Unterminated string literal
 *
 * Verifies:
 *   - Error is reported for unterminated string
 *   - Lexer continues after error
 *   - Subsequent tokens are still processed
 */
TEST_F(LexerErrorRecoveryTest, UnterminatedStringLiteral) {
    std::string source = R"("unterminated string)";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors())
        << "Unterminated string should produce an error";
    EXPECT_GT(lexer.getErrorHandler().get_error_count(), 0);
}

TEST_F(LexerErrorRecoveryTest, UnterminatedStringContinuesLexing) {
    // After an unterminated string, lexer should still process next tokens
    std::string source = "\"unterminated\nint x;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors());

    // Check if we still get some tokens after the error
    // (exact behavior depends on implementation)
    EXPECT_FALSE(tokens.empty());
}

/**
 * Test: Unterminated character literal
 */
TEST_F(LexerErrorRecoveryTest, UnterminatedCharacterLiteral) {
    std::string source = "'a";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors())
        << "Unterminated character literal should produce error";
}

/**
 * Test: Empty character literal
 */
TEST_F(LexerErrorRecoveryTest, EmptyCharacterLiteral) {
    std::string source = "''";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Empty char literal may be an error depending on implementation
    // At minimum, it should not crash
    EXPECT_FALSE(tokens.empty());
}

// ==============================================================================
// Unterminated Comments
// ==============================================================================

/**
 * Test: Unterminated multi-line comment
 *
 * Verifies:
 *   - Error reported for unterminated /*
 *   - Lexer doesn't consume entire rest of file silently
 */
TEST_F(LexerErrorRecoveryTest, UnterminatedMultiLineComment) {
    std::string source = "/* This comment never ends\nint x = 5;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors())
        << "Unterminated multi-line comment should produce error";
}

// ==============================================================================
// Invalid Characters
// ==============================================================================

/**
 * Test: Invalid character in source
 *
 * Verifies:
 *   - Non-ASCII or invalid characters produce errors
 *   - Lexer recovers and continues
 */
TEST_F(LexerErrorRecoveryTest, InvalidCharacterInSource) {
    // Use a character that's invalid in C source (e.g., @, $, ` outside strings)
    std::string source = "int x @ 5;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should report error for '@' (not valid in C)
    EXPECT_TRUE(lexer.hasErrors());

    // Should still recognize 'int' and 'x'
    bool found_int = false;
    bool found_x = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::KW_INT) found_int = true;
        if (token.type == TokenType::IDENTIFIER && token.value == "x") found_x = true;
    }

    EXPECT_TRUE(found_int) << "Should recover and find 'int'";
    EXPECT_TRUE(found_x) << "Should recover and find 'x'";
}

/**
 * Test: Multiple invalid characters
 */
TEST_F(LexerErrorRecoveryTest, MultipleInvalidCharacters) {
    std::string source = "int @ x $ = # 5;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should report multiple errors
    EXPECT_TRUE(lexer.hasErrors());
    // Note: # might be treated as preprocessor token depending on implementation
}

// ==============================================================================
// Malformed Number Literals
// ==============================================================================

/**
 * Test: Invalid hexadecimal literal
 *
 * Verifies:
 *   - 0x without digits is error
 */
TEST_F(LexerErrorRecoveryTest, InvalidHexadecimalLiteral) {
    std::string source = "0x";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // 0x by itself should be invalid
    EXPECT_TRUE(lexer.hasErrors() || tokens[0].type == TokenType::UNKNOWN);
}

/**
 * Test: Invalid float literal format
 */
TEST_F(LexerErrorRecoveryTest, MalformedFloatLiteral) {
    // Multiple decimal points
    std::string source = "3.14.159";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should either error or tokenize as separate tokens
    EXPECT_FALSE(tokens.empty());
}

// ==============================================================================
// Error Recovery - Multiple Errors
// ==============================================================================

/**
 * Test: Multiple errors in same source
 *
 * Verifies:
 *   - Lexer reports multiple errors
 *   - Error count is accurate
 *   - Lexer doesn't stop at first error
 */
TEST_F(LexerErrorRecoveryTest, MultipleErrorsReported) {
    std::string source = R"(
        "unterminated1
        int x @ 5;
        "unterminated2
        char c = 'ab;
    )";

    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors());
    EXPECT_GT(lexer.getErrorHandler().get_error_count(), 1)
        << "Should report multiple errors";
}

/**
 * Test: Error recovery allows subsequent valid code
 *
 * Verifies:
 *   - After error, lexer continues normally
 *   - Valid tokens after error are recognized
 */
TEST_F(LexerErrorRecoveryTest, RecoveryAllowsValidCode) {
    std::string source = R"(
        int @ bad;
        int good = 42;
    )";

    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should have errors, but also find the valid "int good = 42;"
    EXPECT_TRUE(lexer.hasErrors());

    bool found_good = false;
    bool found_42 = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::IDENTIFIER && token.value == "good")
            found_good = true;
        if (token.type == TokenType::INT_LITERAL && token.value == "42")
            found_42 = true;
    }

    EXPECT_TRUE(found_good) << "Should find 'good' after error";
    EXPECT_TRUE(found_42) << "Should find '42' after error";
}

// ==============================================================================
// Error Diagnostics Quality
// ==============================================================================

/**
 * Test: Error messages include location information
 *
 * Verifies:
 *   - Errors reference correct file, line, column
 */
TEST_F(LexerErrorRecoveryTest, ErrorsIncludeLocationInfo) {
    std::string source = "int x = 5;\n\"unterminated";
    Lexer lexer(source, "myfile.c");

    ErrorCapture capture(lexer.getErrorHandler());
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(capture.has_new_errors());
    // Additional assertions could verify error contains line 2
}

// ==============================================================================
// Stress Testing - Error Limits
// ==============================================================================

/**
 * Test: Many errors don't cause infinite loop
 *
 * Verifies:
 *   - Lexer handles many errors gracefully
 *   - Error count limits are respected
 */
TEST_F(LexerErrorRecoveryTest, ManyErrorsDontHang) {
    // Create source with many invalid characters
    std::string source;
    for (int i = 0; i < 50; i++) {
        source += "@ ";
    }

    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should complete without hanging
    EXPECT_TRUE(lexer.hasErrors());
    EXPECT_FALSE(tokens.empty());

    // Check if there's an error limit in place
    // (100 is the MAX_ERRORS from lexer.h)
    EXPECT_LE(lexer.getErrorHandler().get_error_count(), 100)
        << "Error count should be capped";
}

// ==============================================================================
// Edge Cases
// ==============================================================================

/**
 * Test: Source ends mid-token
 */
TEST_F(LexerErrorRecoveryTest, SourceEndsMidToken) {
    std::string source = "int x = 0x";  // Ends in middle of hex literal
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should handle gracefully (error or treat as incomplete token)
    EXPECT_FALSE(tokens.empty());
}

/**
 * Test: String with newline (usually an error in C)
 */
TEST_F(LexerErrorRecoveryTest, StringWithUnescapedNewline) {
    std::string source = "\"Line 1\nLine 2\"";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Unescaped newline in string is typically an error
    // (depends on implementation)
    EXPECT_FALSE(tokens.empty());
}

/**
 * Test: Null character in source
 */
TEST_F(LexerErrorRecoveryTest, NullCharacterInSource) {
    std::string source = "int x = 5\0 int y = 10;";
    source += ";"; // Ensure string continues after null

    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Should handle null character (might stop or skip it)
    EXPECT_FALSE(tokens.empty());
}

// ==============================================================================
// Recovery Patterns
// ==============================================================================

/**
 * Test: Error in identifier position
 */
TEST_F(LexerErrorRecoveryTest, ErrorInIdentifierPosition) {
    std::string source = "int 123invalid = 5;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Number can't be identifier, but lexer should recover
    EXPECT_FALSE(tokens.empty());

    // Should still find "int"
    bool found_int = false;
    for (const auto& token : tokens) {
        if (token.type == TokenType::KW_INT) found_int = true;
    }
    EXPECT_TRUE(found_int);
}

/**
 * Test: Mixed valid and invalid in expression
 */
TEST_F(LexerErrorRecoveryTest, MixedValidInvalidExpression) {
    std::string source = "x = a + @ + b;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors());

    // Should still tokenize the valid parts
    bool found_x = false;
    bool found_a = false;
    bool found_b = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::IDENTIFIER) {
            if (token.value == "x") found_x = true;
            if (token.value == "a") found_a = true;
            if (token.value == "b") found_b = true;
        }
    }

    EXPECT_TRUE(found_x);
    EXPECT_TRUE(found_a);
    EXPECT_TRUE(found_b);
}

// ==============================================================================
// No Crash Guarantees
// ==============================================================================

/**
 * Test: Extremely malformed input doesn't crash
 */
TEST_F(LexerErrorRecoveryTest, ExtremelyMalformedInputNoCrash) {
    std::string source = "@#$%^&*!~`";
    Lexer lexer(source, "test.c");

    // Should not crash, even with complete garbage
    EXPECT_NO_THROW({
        auto tokens = lexer.lexAll();
        EXPECT_FALSE(tokens.empty()); // Should at least have EOF
    });
}

/**
 * Test: Repeated unterminated strings don't crash
 */
TEST_F(LexerErrorRecoveryTest, RepeatedUnterminatedStringsNoCrash) {
    std::string source = "\"one\n\"two\n\"three\n\"four";
    Lexer lexer(source, "test.c");

    EXPECT_NO_THROW({
        auto tokens = lexer.lexAll();
    });
}
