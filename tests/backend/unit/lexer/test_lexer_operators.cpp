/**
 * ==============================================================================
 * Lexer Operator Tokenization Tests
 * ==============================================================================
 *
 * Module Under Test: Lexer (lexer.h, lexer.cpp)
 *
 * Purpose:
 *   Comprehensive testing of operator recognition including:
 *   - Arithmetic operators
 *   - Comparison operators
 *   - Logical operators
 *   - Bitwise operators
 *   - Assignment operators (simple and compound)
 *   - Increment/decrement operators
 *   - Multi-character operator disambiguation (e.g., ++ vs + +)
 *
 * Coverage:
 *   ✓ Single-character operators
 *   ✓ Multi-character operators
 *   ✓ Longest-match operator resolution
 *   ✓ Operator disambiguation
 *   ✓ Operators without spacing
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"

using namespace mycc_test;

class LexerOperatorTest : public ::testing::Test {};

// ==============================================================================
// Arithmetic Operators
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesBasicArithmeticOperators) {
    std::string source = "+ - * / %";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 5);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_PLUS, "+");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_MINUS, "-");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_STAR, "*");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::OP_SLASH, "/");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::OP_MOD, "%");
}

// ==============================================================================
// Comparison Operators
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesComparisonOperators) {
    std::string source = "== != < <= > >=";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 6);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_EQ, "==");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_NE, "!=");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_LT, "<");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::OP_LE, "<=");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::OP_GT, ">");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::OP_GE, ">=");
}

// ==============================================================================
// Logical Operators
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesLogicalOperators) {
    std::string source = "&& || !";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 3);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_AND, "&&");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_OR, "||");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_NOT, "!");
}

// ==============================================================================
// Bitwise Operators
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesBitwiseOperators) {
    std::string source = "& | ^ ~ << >>";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 6);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_BIT_AND, "&");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_BIT_OR, "|");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_BIT_XOR, "^");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::OP_BIT_NOT, "~");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::OP_LSHIFT, "<<");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::OP_RSHIFT, ">>");
}

// ==============================================================================
// Assignment Operators
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesSimpleAssignment) {
    std::string source = "=";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 1);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_ASSIGN, "=");
}

TEST_F(LexerOperatorTest, RecognizesCompoundAssignmentOperators) {
    std::string source = "+= -= *= /= %= &= |= ^= <<= >>=";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 10);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_PLUS_ASSIGN, "+=");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_MINUS_ASSIGN, "-=");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_STAR_ASSIGN, "*=");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::OP_SLASH_ASSIGN, "/=");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::OP_MOD_ASSIGN, "%=");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::OP_AND_ASSIGN, "&=");
    ASSERT_TOKEN_EQ(tokens[6], TokenType::OP_OR_ASSIGN, "|=");
    ASSERT_TOKEN_EQ(tokens[7], TokenType::OP_XOR_ASSIGN, "^=");
    ASSERT_TOKEN_EQ(tokens[8], TokenType::OP_LSHIFT_ASSIGN, "<<=");
    ASSERT_TOKEN_EQ(tokens[9], TokenType::OP_RSHIFT_ASSIGN, ">>=");
}

// ==============================================================================
// Increment/Decrement Operators
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesIncrementDecrement) {
    std::string source = "++ --";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 2);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_INC, "++");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_DEC, "--");
}

// ==============================================================================
// Ternary/Conditional Operator
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesTernaryOperator) {
    std::string source = "?";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 1);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_QUESTION, "?");
}

// ==============================================================================
// Operator Disambiguation Tests (Longest Match)
// ==============================================================================

/**
 * Test: ++ vs + +
 *
 * Verifies:
 *   - "++" is recognized as single OP_INC
 *   - "+ +" (with space) is two OP_PLUS tokens
 */
TEST_F(LexerOperatorTest, DisambiguatesIncrementVsPlusPlus) {
    // Without space: should be ++
    {
        Lexer lexer("++", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_INC, "++");
    }

    // With space: should be + +
    {
        Lexer lexer("+ +", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 2);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_PLUS, "+");
        ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_PLUS, "+");
    }
}

/**
 * Test: == vs = =
 */
TEST_F(LexerOperatorTest, DisambiguatesEqualityVsAssignAssign) {
    // Without space: should be ==
    {
        Lexer lexer("==", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_EQ, "==");
    }

    // With space: should be = =
    {
        Lexer lexer("= =", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 2);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_ASSIGN, "=");
        ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_ASSIGN, "=");
    }
}

/**
 * Test: << vs < <
 */
TEST_F(LexerOperatorTest, DisambiguatesLeftShiftVsLessLess) {
    {
        Lexer lexer("<<", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_LSHIFT, "<<");
    }

    {
        Lexer lexer("< <", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 2);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_LT, "<");
        ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_LT, "<");
    }
}

/**
 * Test: <<= vs << =
 */
TEST_F(LexerOperatorTest, DisambiguatesLeftShiftAssign) {
    {
        Lexer lexer("<<=", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_LSHIFT_ASSIGN, "<<=");
    }

    {
        Lexer lexer("<< =", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 2);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_LSHIFT, "<<");
        ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_ASSIGN, "=");
    }
}

/**
 * Test: && vs & &
 */
TEST_F(LexerOperatorTest, DisambiguatesLogicalAndVsBitwiseAnd) {
    {
        Lexer lexer("&&", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_AND, "&&");
    }

    {
        Lexer lexer("& &", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 2);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_BIT_AND, "&");
        ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_BIT_AND, "&");
    }
}

// ==============================================================================
// Operators in Expressions (No Spacing)
// ==============================================================================

/**
 * Test: Complex expression without spacing
 *
 * Verifies:
 *   - Operators are correctly separated
 *   - Longest match is applied consistently
 */
TEST_F(LexerOperatorTest, OperatorsInExpressionWithoutSpacing) {
    std::string source = "a+b-c*d/e%f";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // Expected: a + b - c * d / e % f (EOF)
    ASSERT_GE(tokens.size(), 12);

    ASSERT_TOKEN_EQ(tokens[0], TokenType::IDENTIFIER, "a");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_PLUS, "+");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::IDENTIFIER, "b");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::OP_MINUS, "-");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::IDENTIFIER, "c");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::OP_STAR, "*");
    ASSERT_TOKEN_EQ(tokens[6], TokenType::IDENTIFIER, "d");
    ASSERT_TOKEN_EQ(tokens[7], TokenType::OP_SLASH, "/");
    ASSERT_TOKEN_EQ(tokens[8], TokenType::IDENTIFIER, "e");
    ASSERT_TOKEN_EQ(tokens[9], TokenType::OP_MOD, "%");
    ASSERT_TOKEN_EQ(tokens[10], TokenType::IDENTIFIER, "f");
}

/**
 * Test: Comparison chain
 */
TEST_F(LexerOperatorTest, ComparisonChain) {
    std::string source = "a<b<=c>d>=e==f!=g";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 14);
    // Verify operators are correctly recognized
    ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_LT, "<");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::OP_LE, "<=");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::OP_GT, ">");
    ASSERT_TOKEN_EQ(tokens[7], TokenType::OP_GE, ">=");
    ASSERT_TOKEN_EQ(tokens[9], TokenType::OP_EQ, "==");
    ASSERT_TOKEN_EQ(tokens[11], TokenType::OP_NE, "!=");
}

/**
 * Test: Increment/decrement in expression
 */
TEST_F(LexerOperatorTest, IncrementDecrementInExpression) {
    std::string source = "++x--";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 3);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_INC, "++");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::IDENTIFIER, "x");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::OP_DEC, "--");
}

// ==============================================================================
// Arrow and Dot Operators (Member Access)
// ==============================================================================

TEST_F(LexerOperatorTest, RecognizesMemberAccessOperators) {
    std::string source = ". ->";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 2);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::DOT, ".");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::ARROW, "->");
}

/**
 * Test: -> vs - >
 */
TEST_F(LexerOperatorTest, DisambiguatesArrowVsMinusGreater) {
    {
        Lexer lexer("->", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::ARROW, "->");
    }

    {
        Lexer lexer("- >", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_GE(tokens.size(), 2);
        ASSERT_TOKEN_EQ(tokens[0], TokenType::OP_MINUS, "-");
        ASSERT_TOKEN_EQ(tokens[1], TokenType::OP_GT, ">");
    }
}

// ==============================================================================
// Realistic Code Snippets
// ==============================================================================

/**
 * Test: Operators in assignment statement
 */
TEST_F(LexerOperatorTest, OperatorsInAssignment) {
    std::string source = "result = (a + b) * (c - d) / e;";
    auto tokens = lex_without_errors(source);

    bool found_assign = false;
    bool found_plus = false;
    bool found_star = false;
    bool found_minus = false;
    bool found_slash = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::OP_ASSIGN) found_assign = true;
        if (token.type == TokenType::OP_PLUS) found_plus = true;
        if (token.type == TokenType::OP_STAR) found_star = true;
        if (token.type == TokenType::OP_MINUS) found_minus = true;
        if (token.type == TokenType::OP_SLASH) found_slash = true;
    }

    EXPECT_TRUE(found_assign);
    EXPECT_TRUE(found_plus);
    EXPECT_TRUE(found_star);
    EXPECT_TRUE(found_minus);
    EXPECT_TRUE(found_slash);
}

/**
 * Test: Operators in conditional
 */
TEST_F(LexerOperatorTest, OperatorsInConditional) {
    std::string source = "if (x >= 0 && y != NULL)";
    auto tokens = lex_without_errors(source);

    bool found_ge = false;
    bool found_and = false;
    bool found_ne = false;

    for (const auto& token : tokens) {
        if (token.type == TokenType::OP_GE) found_ge = true;
        if (token.type == TokenType::OP_AND) found_and = true;
        if (token.type == TokenType::OP_NE) found_ne = true;
    }

    EXPECT_TRUE(found_ge);
    EXPECT_TRUE(found_and);
    EXPECT_TRUE(found_ne);
}
