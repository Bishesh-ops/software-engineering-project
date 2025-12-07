/**
 * ==============================================================================
 * Lexer Literal Tokenization Tests
 * ==============================================================================
 *
 * Module Under Test: Lexer (lexer.h, lexer.cpp)
 *
 * Purpose:
 *   Comprehensive testing of literal value recognition:
 *   - Integer literals (decimal, hexadecimal, octal)
 *   - Floating-point literals
 *   - Character literals (with escape sequences)
 *   - String literals (with escape sequences)
 *
 * Coverage:
 *   ✓ Decimal integers
 *   ✓ Hexadecimal integers (0x prefix)
 *   ✓ Octal integers (0 prefix)
 *   ✓ Floating-point (with/without scientific notation)
 *   ✓ Character literals and escape sequences
 *   ✓ String literals and escape sequences
 *   ✓ Edge cases (empty strings, long literals)
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"

using namespace mycc_test;

class LexerLiteralsTest : public ::testing::Test {};

// ==============================================================================
// Integer Literals - Decimal
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesDecimalIntegers) {
    std::string source = "0 1 42 123 9999";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 5);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::INT_LITERAL, "0");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::INT_LITERAL, "1");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::INT_LITERAL, "42");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::INT_LITERAL, "123");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::INT_LITERAL, "9999");
}

TEST_F(LexerLiteralsTest, RecognizesLargeDecimalIntegers) {
    std::string source = "2147483647 4294967295";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 2);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::INT_LITERAL, "2147483647");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::INT_LITERAL, "4294967295");
}

// ==============================================================================
// Integer Literals - Hexadecimal
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesHexadecimalIntegers) {
    std::string source = "0x0 0x1 0xA 0xF 0xFF 0xDEADBEEF 0xabcdef";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 7);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::INT_LITERAL, "0x0");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::INT_LITERAL, "0x1");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::INT_LITERAL, "0xA");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::INT_LITERAL, "0xF");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::INT_LITERAL, "0xFF");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::INT_LITERAL, "0xDEADBEEF");
}

TEST_F(LexerLiteralsTest, RecognizesHexadecimalLowerAndUpperCase) {
    std::string source = "0xabcd 0xABCD 0xAbCd";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 3);
    // All should be recognized as hex literals
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::INT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::INT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[2], TokenType::INT_LITERAL);
}

// ==============================================================================
// Integer Literals - Octal
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesOctalIntegers) {
    std::string source = "00 01 07 010 0777";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 5);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::INT_LITERAL, "00");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::INT_LITERAL, "01");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::INT_LITERAL, "07");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::INT_LITERAL, "010");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::INT_LITERAL, "0777");
}

// ==============================================================================
// Floating-Point Literals
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesFloatingPointLiterals) {
    std::string source = "0.0 1.0 3.14 123.456";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::FLOAT_LITERAL, "0.0");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::FLOAT_LITERAL, "1.0");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::FLOAT_LITERAL, "3.14");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::FLOAT_LITERAL, "123.456");
}

TEST_F(LexerLiteralsTest, RecognizesFloatingPointWithTrailingF) {
    std::string source = "3.14f 2.718f";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 2);
    // Should recognize as float literals (implementation dependent)
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::FLOAT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::FLOAT_LITERAL);
}

TEST_F(LexerLiteralsTest, RecognizesFloatingPointScientificNotation) {
    std::string source = "1e10 1E10 3.14e-2 2.5E+5";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::FLOAT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::FLOAT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[2], TokenType::FLOAT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[3], TokenType::FLOAT_LITERAL);
}

TEST_F(LexerLiteralsTest, RecognizesFloatingPointEdgeCases) {
    std::string source = ".5 0. 1.0e0";
    auto tokens = lex_without_errors(source);

    // All should produce some valid token sequence
    ASSERT_FALSE(tokens.empty());
}

// ==============================================================================
// Character Literals - Basic
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesSimpleCharacterLiterals) {
    std::string source = "'a' 'Z' '0' ' '";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::CHAR_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::CHAR_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[2], TokenType::CHAR_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[3], TokenType::CHAR_LITERAL);
}

// ==============================================================================
// Character Literals - Escape Sequences
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesCharacterEscapeSequences) {
    std::string source = R"('\n' '\t' '\r' '\\' '\'' '\"')";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 6);
    // All should be character literals
    for (int i = 0; i < 6; i++) {
        ASSERT_TOKEN_TYPE(tokens[i], TokenType::CHAR_LITERAL)
            << "Token " << i << " should be char literal";
    }
}

TEST_F(LexerLiteralsTest, RecognizesCharacterOctalEscapes) {
    std::string source = R"('\0' '\101' '\177')";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 3);
    for (const auto& token : tokens) {
        if (token.type != TokenType::EOF_TOKEN) {
            ASSERT_TOKEN_TYPE(token, TokenType::CHAR_LITERAL);
        }
    }
}

TEST_F(LexerLiteralsTest, RecognizesCharacterHexEscapes) {
    std::string source = R"('\x00' '\x41' '\xFF')";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 3);
    // Should recognize hex escapes
    for (int i = 0; i < 3; i++) {
        ASSERT_TOKEN_TYPE(tokens[i], TokenType::CHAR_LITERAL);
    }
}

// ==============================================================================
// String Literals - Basic
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesSimpleStringLiterals) {
    std::string source = R"("hello" "world" "123" "")";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::STRING_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::STRING_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[2], TokenType::STRING_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[3], TokenType::STRING_LITERAL);
}

TEST_F(LexerLiteralsTest, RecognizesEmptyString) {
    std::string source = R"("")";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 1);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::STRING_LITERAL);
}

TEST_F(LexerLiteralsTest, RecognizesStringWithSpaces) {
    std::string source = R"("hello world" "  spaces  ")";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 2);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::STRING_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::STRING_LITERAL);
}

// ==============================================================================
// String Literals - Escape Sequences
// ==============================================================================

TEST_F(LexerLiteralsTest, RecognizesStringEscapeSequences) {
    std::string source = R"("Line 1\nLine 2" "Tab\there" "Quote: \"" "Backslash: \\")";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    for (int i = 0; i < 4; i++) {
        ASSERT_TOKEN_TYPE(tokens[i], TokenType::STRING_LITERAL);
    }
}

TEST_F(LexerLiteralsTest, RecognizesStringWithAllEscapes) {
    // Test string with multiple escape types
    std::string source = R"("Test: \n \t \r \\ \" \' \0")";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 1);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::STRING_LITERAL);
}

// ==============================================================================
// Literals in Context
// ==============================================================================

TEST_F(LexerLiteralsTest, LiteralsInVariableDeclaration) {
    std::string source = R"(int x = 42; float pi = 3.14; char c = 'A';)";
    auto tokens = lex_without_errors(source);

    // Verify we have int, float, and char literals
    bool found_int = false;
    bool found_float = false;
    bool found_char = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::INT_LITERAL && token.value == "42")
            found_int = true;
        if (token.type == TokenType::FLOAT_LITERAL && token.value == "3.14")
            found_float = true;
        if (token.type == TokenType::CHAR_LITERAL)
            found_char = true;
    }

    EXPECT_TRUE(found_int);
    EXPECT_TRUE(found_float);
    EXPECT_TRUE(found_char);
}

TEST_F(LexerLiteralsTest, LiteralsInExpression) {
    std::string source = "result = 100 + 50 * 2.5 - 10;";
    auto tokens = lex_without_errors(source);

    int int_literal_count = 0;
    int float_literal_count = 0;

    for (const auto& token : tokens) {
        if (token.type == TokenType::INT_LITERAL) int_literal_count++;
        if (token.type == TokenType::FLOAT_LITERAL) float_literal_count++;
    }

    EXPECT_EQ(int_literal_count, 3) << "Should have 100, 50, 10";
    EXPECT_EQ(float_literal_count, 1) << "Should have 2.5";
}

TEST_F(LexerLiteralsTest, StringLiteralsInFunctionCall) {
    std::string source = R"(printf("Hello, %s!\n", "World");)";
    auto tokens = lex_without_errors(source);

    int string_count = 0;
    for (const auto& token : tokens) {
        if (token.type == TokenType::STRING_LITERAL) string_count++;
    }

    EXPECT_EQ(string_count, 2) << "Should have two string literals";
}

// ==============================================================================
// Mixed Literals
// ==============================================================================

TEST_F(LexerLiteralsTest, MixedLiteralsInSameStatement) {
    std::string source = R"(int arr[] = {1, 2, 3}; char str[] = "abc"; float f = 1.5;)";
    auto tokens = lex_without_errors(source);

    bool has_int = false;
    bool has_float = false;
    bool has_string = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::INT_LITERAL) has_int = true;
        if (token.type == TokenType::FLOAT_LITERAL) has_float = true;
        if (token.type == TokenType::STRING_LITERAL) has_string = true;
    }

    EXPECT_TRUE(has_int);
    EXPECT_TRUE(has_float);
    EXPECT_TRUE(has_string);
}

// ==============================================================================
// Edge Cases
// ==============================================================================

TEST_F(LexerLiteralsTest, AdjacentLiterals) {
    // Literals separated only by whitespace
    std::string source = "42 3.14 'x' \"str\"";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::INT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::FLOAT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[2], TokenType::CHAR_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[3], TokenType::STRING_LITERAL);
}

TEST_F(LexerLiteralsTest, LongStringLiteral) {
    std::string long_content(1000, 'a');
    std::string source = "\"" + long_content + "\"";

    auto tokens = lex_without_errors(source);
    ASSERT_GE(tokens.size(), 1);
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::STRING_LITERAL);
}

TEST_F(LexerLiteralsTest, ZeroVariants) {
    // Different ways to represent zero
    std::string source = "0 0x0 00 0.0";
    auto tokens = lex_without_errors(source);

    ASSERT_GE(tokens.size(), 4);
    // First three should be int literals
    ASSERT_TOKEN_TYPE(tokens[0], TokenType::INT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[1], TokenType::INT_LITERAL);
    ASSERT_TOKEN_TYPE(tokens[2], TokenType::INT_LITERAL);
    // Last should be float
    ASSERT_TOKEN_TYPE(tokens[3], TokenType::FLOAT_LITERAL);
}
