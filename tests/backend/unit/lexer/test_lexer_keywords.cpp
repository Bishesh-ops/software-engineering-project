/**
 * ==============================================================================
 * Lexer Keyword Recognition Tests
 * ==============================================================================
 *
 * Module Under Test: Lexer (lexer.h, lexer.cpp)
 *
 * Purpose:
 *   Comprehensive testing of C keyword recognition and differentiation from
 *   identifiers. Ensures all C89/C90 keywords are properly tokenized.
 *
 * Coverage:
 *   ✓ All 32 C89 keywords
 *   ✓ Case sensitivity (keywords vs identifiers)
 *   ✓ Keyword-like identifiers (e.g., "int_value", "if_stmt")
 *   ✓ Keywords in different contexts
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"

using namespace mycc_test;

// ==============================================================================
// Keyword Recognition Test Fixture
// ==============================================================================

class LexerKeywordTest : public ::testing::Test {
protected:
    /**
     * Helper to verify keyword tokenization
     */
    void ExpectKeyword(const std::string& keyword, TokenType expected_type) {
        Lexer lexer(keyword, "test.c");
        auto tokens = lexer.lexAll();

        ASSERT_GE(tokens.size(), 1);
        ASSERT_TOKEN_EQ(tokens[0], expected_type, keyword);
        EXPECT_FALSE(lexer.hasErrors());
    }
};

// ==============================================================================
// Individual Keyword Tests (C89 Standard - 32 Keywords)
// ==============================================================================

TEST_F(LexerKeywordTest, RecognizesAuto) {
    ExpectKeyword("auto", TokenType::KW_AUTO);
}

TEST_F(LexerKeywordTest, RecognizesBreak) {
    ExpectKeyword("break", TokenType::KW_BREAK);
}

TEST_F(LexerKeywordTest, RecognizesCase) {
    ExpectKeyword("case", TokenType::KW_CASE);
}

TEST_F(LexerKeywordTest, RecognizesChar) {
    ExpectKeyword("char", TokenType::KW_CHAR);
}

TEST_F(LexerKeywordTest, RecognizesConst) {
    ExpectKeyword("const", TokenType::KW_CONST);
}

TEST_F(LexerKeywordTest, RecognizesContinue) {
    ExpectKeyword("continue", TokenType::KW_CONTINUE);
}

TEST_F(LexerKeywordTest, RecognizesDefault) {
    ExpectKeyword("default", TokenType::KW_DEFAULT);
}

TEST_F(LexerKeywordTest, RecognizesDo) {
    ExpectKeyword("do", TokenType::KW_DO);
}

TEST_F(LexerKeywordTest, RecognizesDouble) {
    ExpectKeyword("double", TokenType::KW_DOUBLE);
}

TEST_F(LexerKeywordTest, RecognizesElse) {
    ExpectKeyword("else", TokenType::KW_ELSE);
}

TEST_F(LexerKeywordTest, RecognizesEnum) {
    ExpectKeyword("enum", TokenType::KW_ENUM);
}

TEST_F(LexerKeywordTest, RecognizesExtern) {
    ExpectKeyword("extern", TokenType::KW_EXTERN);
}

TEST_F(LexerKeywordTest, RecognizesFloat) {
    ExpectKeyword("float", TokenType::KW_FLOAT);
}

TEST_F(LexerKeywordTest, RecognizesFor) {
    ExpectKeyword("for", TokenType::KW_FOR);
}

TEST_F(LexerKeywordTest, RecognizesGoto) {
    ExpectKeyword("goto", TokenType::KW_GOTO);
}

TEST_F(LexerKeywordTest, RecognizesIf) {
    ExpectKeyword("if", TokenType::KW_IF);
}

TEST_F(LexerKeywordTest, RecognizesInt) {
    ExpectKeyword("int", TokenType::KW_INT);
}

TEST_F(LexerKeywordTest, RecognizesLong) {
    ExpectKeyword("long", TokenType::KW_LONG);
}

TEST_F(LexerKeywordTest, RecognizesRegister) {
    ExpectKeyword("register", TokenType::KW_REGISTER);
}

TEST_F(LexerKeywordTest, RecognizesReturn) {
    ExpectKeyword("return", TokenType::KW_RETURN);
}

TEST_F(LexerKeywordTest, RecognizesShort) {
    ExpectKeyword("short", TokenType::KW_SHORT);
}

TEST_F(LexerKeywordTest, RecognizesSigned) {
    ExpectKeyword("signed", TokenType::KW_SIGNED);
}

TEST_F(LexerKeywordTest, RecognizesSizeof) {
    ExpectKeyword("sizeof", TokenType::KW_SIZEOF);
}

TEST_F(LexerKeywordTest, RecognizesStatic) {
    ExpectKeyword("static", TokenType::KW_STATIC);
}

TEST_F(LexerKeywordTest, RecognizesStruct) {
    ExpectKeyword("struct", TokenType::KW_STRUCT);
}

TEST_F(LexerKeywordTest, RecognizesSwitch) {
    ExpectKeyword("switch", TokenType::KW_SWITCH);
}

TEST_F(LexerKeywordTest, RecognizesTypedef) {
    ExpectKeyword("typedef", TokenType::KW_TYPEDEF);
}

TEST_F(LexerKeywordTest, RecognizesUnion) {
    ExpectKeyword("union", TokenType::KW_UNION);
}

TEST_F(LexerKeywordTest, RecognizesUnsigned) {
    ExpectKeyword("unsigned", TokenType::KW_UNSIGNED);
}

TEST_F(LexerKeywordTest, RecognizesVoid) {
    ExpectKeyword("void", TokenType::KW_VOID);
}

TEST_F(LexerKeywordTest, RecognizesVolatile) {
    ExpectKeyword("volatile", TokenType::KW_VOLATILE);
}

TEST_F(LexerKeywordTest, RecognizesWhile) {
    ExpectKeyword("while", TokenType::KW_WHILE);
}

// ==============================================================================
// Case Sensitivity Tests
// ==============================================================================

/**
 * Test: Keywords are case-sensitive
 *
 * Verifies:
 *   - "int" is a keyword
 *   - "Int", "INT", "iNt" are identifiers
 */
TEST_F(LexerKeywordTest, KeywordsAreCaseSensitive) {
    // Lowercase should be keyword
    {
        Lexer lexer("int", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_TOKEN_TYPE(tokens[0], TokenType::KW_INT);
    }

    // Uppercase should be identifier
    {
        Lexer lexer("INT", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_TOKEN_TYPE(tokens[0], TokenType::IDENTIFIER);
    }

    // Mixed case should be identifier
    {
        Lexer lexer("Int", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_TOKEN_TYPE(tokens[0], TokenType::IDENTIFIER);
    }

    {
        Lexer lexer("iNt", "test.c");
        auto tokens = lexer.lexAll();
        ASSERT_TOKEN_TYPE(tokens[0], TokenType::IDENTIFIER);
    }
}

// ==============================================================================
// Keyword vs Identifier Differentiation
// ==============================================================================

/**
 * Test: Identifiers containing keywords as substrings
 *
 * Verifies:
 *   - "int_value" is an identifier, not keyword
 *   - "if_stmt" is an identifier
 *   - Keyword must be a complete token
 */
TEST_F(LexerKeywordTest, IdentifiersContainingKeywords) {
    std::string source = "int_value if_stmt return_code while_loop";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    // All should be identifiers
    ASSERT_GE(tokens.size(), 4);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::IDENTIFIER, "int_value");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::IDENTIFIER, "if_stmt");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::IDENTIFIER, "return_code");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::IDENTIFIER, "while_loop");
}

/**
 * Test: Keywords with underscores prefix/suffix
 *
 * Verifies:
 *   - "_int" is identifier
 *   - "int_" is identifier
 */
TEST_F(LexerKeywordTest, KeywordsWithUnderscores) {
    std::string source = "_int int_ _return_ __while__";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 4);
    for (int i = 0; i < 4; i++) {
        ASSERT_TOKEN_TYPE(tokens[i], TokenType::IDENTIFIER)
            << "Token " << i << " should be identifier";
    }
}

// ==============================================================================
// Keywords in Context
// ==============================================================================

/**
 * Test: Multiple keywords in sequence
 *
 * Verifies:
 *   - Keywords separated by whitespace
 *   - Each keyword independently recognized
 */
TEST_F(LexerKeywordTest, MultipleKeywordsInSequence) {
    std::string source = "unsigned long int";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 3);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_UNSIGNED, "unsigned");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::KW_LONG, "long");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::KW_INT, "int");
}

/**
 * Test: Keywords in realistic declaration
 *
 * Verifies:
 *   - Keywords mixed with identifiers
 *   - Proper context handling
 */
TEST_F(LexerKeywordTest, KeywordsInDeclaration) {
    std::string source = "static const int MAX_SIZE = 100;";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 6);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_STATIC, "static");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::KW_CONST, "const");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::KW_INT, "int");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::IDENTIFIER, "MAX_SIZE");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::OP_ASSIGN, "=");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::INT_LITERAL, "100");
}

/**
 * Test: Keywords in control flow
 *
 * Verifies:
 *   - Keywords in if/while/for statements
 */
TEST_F(LexerKeywordTest, KeywordsInControlFlow) {
    std::string source = "if while for return break continue";
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 6);
    ASSERT_TOKEN_EQ(tokens[0], TokenType::KW_IF, "if");
    ASSERT_TOKEN_EQ(tokens[1], TokenType::KW_WHILE, "while");
    ASSERT_TOKEN_EQ(tokens[2], TokenType::KW_FOR, "for");
    ASSERT_TOKEN_EQ(tokens[3], TokenType::KW_RETURN, "return");
    ASSERT_TOKEN_EQ(tokens[4], TokenType::KW_BREAK, "break");
    ASSERT_TOKEN_EQ(tokens[5], TokenType::KW_CONTINUE, "continue");
}

// ==============================================================================
// Parameterized Test: All Keywords Comprehensive
// ==============================================================================

/**
 * Structure mapping keywords to their token types
 */
struct KeywordMapping {
    std::string keyword;
    TokenType token_type;
};

class LexerAllKeywordsTest : public ::testing::TestWithParam<KeywordMapping> {
};

/**
 * Parameterized test for all 32 C89 keywords
 */
TEST_P(LexerAllKeywordsTest, RecognizesKeyword) {
    auto param = GetParam();
    Lexer lexer(param.keyword, "test.c");
    auto tokens = lexer.lexAll();

    ASSERT_GE(tokens.size(), 1);
    ASSERT_TOKEN_EQ(tokens[0], param.token_type, param.keyword);
    EXPECT_FALSE(lexer.hasErrors());
}

// Instantiate parameterized test with all keywords
INSTANTIATE_TEST_SUITE_P(
    AllC89Keywords,
    LexerAllKeywordsTest,
    ::testing::Values(
        KeywordMapping{"auto", TokenType::KW_AUTO},
        KeywordMapping{"break", TokenType::KW_BREAK},
        KeywordMapping{"case", TokenType::KW_CASE},
        KeywordMapping{"char", TokenType::KW_CHAR},
        KeywordMapping{"const", TokenType::KW_CONST},
        KeywordMapping{"continue", TokenType::KW_CONTINUE},
        KeywordMapping{"default", TokenType::KW_DEFAULT},
        KeywordMapping{"do", TokenType::KW_DO},
        KeywordMapping{"double", TokenType::KW_DOUBLE},
        KeywordMapping{"else", TokenType::KW_ELSE},
        KeywordMapping{"enum", TokenType::KW_ENUM},
        KeywordMapping{"extern", TokenType::KW_EXTERN},
        KeywordMapping{"float", TokenType::KW_FLOAT},
        KeywordMapping{"for", TokenType::KW_FOR},
        KeywordMapping{"goto", TokenType::KW_GOTO},
        KeywordMapping{"if", TokenType::KW_IF},
        KeywordMapping{"int", TokenType::KW_INT},
        KeywordMapping{"long", TokenType::KW_LONG},
        KeywordMapping{"register", TokenType::KW_REGISTER},
        KeywordMapping{"return", TokenType::KW_RETURN},
        KeywordMapping{"short", TokenType::KW_SHORT},
        KeywordMapping{"signed", TokenType::KW_SIGNED},
        KeywordMapping{"sizeof", TokenType::KW_SIZEOF},
        KeywordMapping{"static", TokenType::KW_STATIC},
        KeywordMapping{"struct", TokenType::KW_STRUCT},
        KeywordMapping{"switch", TokenType::KW_SWITCH},
        KeywordMapping{"typedef", TokenType::KW_TYPEDEF},
        KeywordMapping{"union", TokenType::KW_UNION},
        KeywordMapping{"unsigned", TokenType::KW_UNSIGNED},
        KeywordMapping{"void", TokenType::KW_VOID},
        KeywordMapping{"volatile", TokenType::KW_VOLATILE},
        KeywordMapping{"while", TokenType::KW_WHILE}
    )
);
