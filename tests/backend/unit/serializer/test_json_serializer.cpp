#include <gtest/gtest.h>
#include "json_serializer.h"
#include "lexer.h"
#include "parser.h"
#include "test_helpers.h"
#include <string>

// ============================================================================
// Test Fixture for JSON Serializer
// ============================================================================
class JSONSerializerTest : public ::testing::Test {
protected:
    // Helper: Check if JSON contains a field
    bool jsonContains(const std::string& json, const std::string& field) {
        return json.find(field) != std::string::npos;
    }

    // Helper: Check if JSON is valid (basic check for balanced braces/brackets)
    bool isValidJSON(const std::string& json) {
        int braceCount = 0;
        int bracketCount = 0;

        for (char c : json) {
            if (c == '{') braceCount++;
            if (c == '}') braceCount--;
            if (c == '[') bracketCount++;
            if (c == ']') bracketCount--;

            // Check for negative counts (closing before opening)
            if (braceCount < 0 || bracketCount < 0) return false;
        }

        return braceCount == 0 && bracketCount == 0;
    }

    // Helper: Count occurrences of a substring
    int countOccurrences(const std::string& str, const std::string& substr) {
        int count = 0;
        size_t pos = 0;
        while ((pos = str.find(substr, pos)) != std::string::npos) {
            count++;
            pos += substr.length();
        }
        return count;
    }
};

// ============================================================================
// Utility Method Tests
// ============================================================================

TEST_F(JSONSerializerTest, EscapesQuotesInStrings) {
    std::string input = "Hello \"World\"";
    std::string escaped = JSONSerializer::escapeJSON(input);
    EXPECT_TRUE(escaped.find("\\\"") != std::string::npos);
}

TEST_F(JSONSerializerTest, EscapesBackslashes) {
    std::string input = "Path\\To\\File";
    std::string escaped = JSONSerializer::escapeJSON(input);
    EXPECT_TRUE(escaped.find("\\\\") != std::string::npos);
}

TEST_F(JSONSerializerTest, EscapesNewlines) {
    std::string input = "Line1\nLine2";
    std::string escaped = JSONSerializer::escapeJSON(input);
    EXPECT_TRUE(escaped.find("\\n") != std::string::npos);
    EXPECT_FALSE(escaped.find("\n") != std::string::npos);  // No actual newline
}

TEST_F(JSONSerializerTest, EscapesTabs) {
    std::string input = "Col1\tCol2";
    std::string escaped = JSONSerializer::escapeJSON(input);
    EXPECT_TRUE(escaped.find("\\t") != std::string::npos);
}

TEST_F(JSONSerializerTest, HandlesEmptyString) {
    std::string input = "";
    std::string escaped = JSONSerializer::escapeJSON(input);
    EXPECT_EQ(escaped, "");
}

// ============================================================================
// Token Type Conversion Tests
// ============================================================================

TEST_F(JSONSerializerTest, ConvertsKeywordTokenTypes) {
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::KW_INT), "KW_INT");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::KW_RETURN), "KW_RETURN");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::KW_IF), "KW_IF");
}

TEST_F(JSONSerializerTest, ConvertsOperatorTokenTypes) {
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::OP_PLUS), "OP_PLUS");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::OP_EQ), "OP_EQ");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::OP_ASSIGN), "OP_ASSIGN");
}

TEST_F(JSONSerializerTest, ConvertsLiteralTokenTypes) {
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::INT_LITERAL), "INT_LITERAL");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::STRING_LITERAL), "STRING_LITERAL");
}

TEST_F(JSONSerializerTest, ConvertsPunctuationTokenTypes) {
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::SEMICOLON), "SEMICOLON");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::LBRACE), "LBRACE");
    EXPECT_EQ(JSONSerializer::tokenTypeToString(TokenType::RPAREN), "RPAREN");
}

// ============================================================================
// Single Token Serialization Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesSingleToken) {
    Token token{TokenType::IDENTIFIER, "x", "test.c", 1, 5};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "IDENTIFIER"));
    EXPECT_TRUE(jsonContains(json, "\"x\""));
    EXPECT_TRUE(jsonContains(json, "\"line\":1"));
    EXPECT_TRUE(jsonContains(json, "\"column\":5"));
}

TEST_F(JSONSerializerTest, SerializesKeywordToken) {
    Token token{TokenType::KW_INT, "int", "test.c", 2, 1};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "KW_INT"));
    EXPECT_TRUE(jsonContains(json, "\"int\""));
}

TEST_F(JSONSerializerTest, SerializesOperatorToken) {
    Token token{TokenType::OP_PLUS, "+", "test.c", 3, 10};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "OP_PLUS"));
    EXPECT_TRUE(jsonContains(json, "\"+\""));
}

TEST_F(JSONSerializerTest, SerializesStringLiteralToken) {
    Token token{TokenType::STRING_LITERAL, "\"hello\"", "test.c", 4, 2};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "STRING_LITERAL"));
}

TEST_F(JSONSerializerTest, TokenJSONContainsAllFields) {
    Token token{TokenType::IDENTIFIER, "myVar", "main.c", 10, 15};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(jsonContains(json, "\"type\""));
    EXPECT_TRUE(jsonContains(json, "\"value\""));
    EXPECT_TRUE(jsonContains(json, "\"line\""));
    EXPECT_TRUE(jsonContains(json, "\"column\""));
    EXPECT_TRUE(jsonContains(json, "\"filename\""));
}

// ============================================================================
// Multiple Tokens Serialization Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesEmptyTokenList) {
    std::vector<Token> tokens;
    std::string json = JSONSerializer::serializeTokens(tokens);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_EQ(json, "[]");
}

TEST_F(JSONSerializerTest, SerializesMultipleTokens) {
    std::vector<Token> tokens = {
        {TokenType::KW_INT, "int", "test.c", 1, 1},
        {TokenType::IDENTIFIER, "x", "test.c", 1, 5},
        {TokenType::SEMICOLON, ";", "test.c", 1, 6}
    };

    std::string json = JSONSerializer::serializeTokens(tokens);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "KW_INT"));
    EXPECT_TRUE(jsonContains(json, "IDENTIFIER"));
    EXPECT_TRUE(jsonContains(json, "SEMICOLON"));
    EXPECT_EQ(countOccurrences(json, "\"type\""), 3);
}

TEST_F(JSONSerializerTest, TokenArrayIsWellFormed) {
    std::vector<Token> tokens = {
        {TokenType::IDENTIFIER, "a", "test.c", 1, 1},
        {TokenType::OP_PLUS, "+", "test.c", 1, 3}
    };

    std::string json = JSONSerializer::serializeTokens(tokens);

    EXPECT_TRUE(json.front() == '[');
    EXPECT_TRUE(json.back() == ']');
    EXPECT_TRUE(isValidJSON(json));
}

// ============================================================================
// Hex Dump Tests
// ============================================================================

TEST_F(JSONSerializerTest, CreatesHexDumpOfBinaryData) {
    const char data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};  // "Hello"
    std::string json = JSONSerializer::hexDump(data, 5);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "\"size\":5"));
    EXPECT_TRUE(jsonContains(json, "\"hex\""));
    EXPECT_TRUE(jsonContains(json, "\"ascii\""));
}

TEST_F(JSONSerializerTest, HexDumpContainsHexRepresentation) {
    const unsigned char data[] = {0xFF, 0x00, 0xAB};
    std::string json = JSONSerializer::hexDump(data, 3);

    EXPECT_TRUE(jsonContains(json, "ff"));
    EXPECT_TRUE(jsonContains(json, "00"));
    EXPECT_TRUE(jsonContains(json, "ab"));
}

TEST_F(JSONSerializerTest, HexDumpHandlesEmptyData) {
    std::string json = JSONSerializer::hexDump(nullptr, 0);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "\"size\":0"));
}

TEST_F(JSONSerializerTest, HexDumpString) {
    std::string input = "ABC";
    std::string json = JSONSerializer::hexDumpString(input);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "\"size\":3"));
    EXPECT_TRUE(jsonContains(json, "\"ascii\":\"ABC\""));
}

TEST_F(JSONSerializerTest, HexDumpNonPrintableCharacters) {
    std::string input = "A\nB";  // Contains non-printable newline
    std::string json = JSONSerializer::hexDumpString(input);

    EXPECT_TRUE(isValidJSON(json));
    // Newline should be represented as '.' in ASCII view
    EXPECT_TRUE(jsonContains(json, "\"ascii\":\"A.B\""));
}

// ============================================================================
// AST Node Type Conversion Tests
// ============================================================================

TEST_F(JSONSerializerTest, ConvertsExpressionNodeTypes) {
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::BINARY_EXPR), "BinaryExpr");
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::LITERAL_EXPR), "LiteralExpr");
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::IDENTIFIER_EXPR), "IdentifierExpr");
}

TEST_F(JSONSerializerTest, ConvertsStatementNodeTypes) {
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::IF_STMT), "IfStmt");
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::RETURN_STMT), "ReturnStmt");
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::WHILE_STMT), "WhileStmt");
}

TEST_F(JSONSerializerTest, ConvertsDeclarationNodeTypes) {
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::VAR_DECL), "VarDecl");
    EXPECT_EQ(JSONSerializer::nodeTypeToString(ASTNodeType::FUNCTION_DECL), "FunctionDecl");
}

// ============================================================================
// AST Serialization - Literal Expression Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesLiteralExpression) {
    auto expr = mycc_test::parse_expression_without_errors("42");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "LiteralExpr"));
    EXPECT_TRUE(jsonContains(json, "\"value\":\"42\""));
}

TEST_F(JSONSerializerTest, SerializesStringLiteral) {
    auto expr = mycc_test::parse_expression_without_errors("\"hello\"");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "LiteralExpr"));
}

// ============================================================================
// AST Serialization - Identifier Expression Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesIdentifierExpression) {
    auto expr = mycc_test::parse_expression_without_errors("myVariable");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "IdentifierExpr"));
    EXPECT_TRUE(jsonContains(json, "\"name\":\"myVariable\""));
}

// ============================================================================
// AST Serialization - Binary Expression Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesBinaryExpression) {
    auto expr = mycc_test::parse_expression_without_errors("a + b");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "BinaryExpr"));
    EXPECT_TRUE(jsonContains(json, "\"operator\":\"+\""));
    EXPECT_TRUE(jsonContains(json, "\"left\""));
    EXPECT_TRUE(jsonContains(json, "\"right\""));
}

TEST_F(JSONSerializerTest, SerializesNestedBinaryExpression) {
    auto expr = mycc_test::parse_expression_without_errors("a + b * c");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    // Should have multiple BinaryExpr nodes for nested structure
    EXPECT_GE(countOccurrences(json, "BinaryExpr"), 2);
}

TEST_F(JSONSerializerTest, BinaryExpressionContainsOperands) {
    auto expr = mycc_test::parse_expression_without_errors("x - y");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(jsonContains(json, "\"left\""));
    EXPECT_TRUE(jsonContains(json, "\"right\""));
    EXPECT_TRUE(jsonContains(json, "\"operator\":\"-\""));
}

// ============================================================================
// AST Serialization - Unary Expression Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesUnaryExpression) {
    auto expr = mycc_test::parse_expression_without_errors("-x");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "UnaryExpr"));
    EXPECT_TRUE(jsonContains(json, "\"operator\":\"-\""));
    EXPECT_TRUE(jsonContains(json, "\"operand\""));
}

TEST_F(JSONSerializerTest, SerializesLogicalNotExpression) {
    auto expr = mycc_test::parse_expression_without_errors("!flag");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "UnaryExpr"));
    EXPECT_TRUE(jsonContains(json, "\"operator\":\"!\""));
}

// ============================================================================
// AST Serialization - Function Call Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesFunctionCall) {
    auto expr = mycc_test::parse_expression_without_errors("foo()");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "CallExpr"));
    EXPECT_TRUE(jsonContains(json, "\"callee\""));
    EXPECT_TRUE(jsonContains(json, "\"arguments\""));
}

TEST_F(JSONSerializerTest, SerializesFunctionCallWithArguments) {
    auto expr = mycc_test::parse_expression_without_errors("add(1, 2)");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "CallExpr"));
    EXPECT_TRUE(jsonContains(json, "\"arguments\":["));
}

// ============================================================================
// AST Serialization - Null Handling Tests
// ============================================================================

TEST_F(JSONSerializerTest, HandlesNullExpression) {
    std::string json = JSONSerializer::serializeExpression(nullptr);
    EXPECT_EQ(json, "null");
}

TEST_F(JSONSerializerTest, HandlesNullStatement) {
    std::string json = JSONSerializer::serializeStatement(nullptr);
    EXPECT_EQ(json, "null");
}

TEST_F(JSONSerializerTest, HandlesNullDeclaration) {
    std::string json = JSONSerializer::serializeDeclaration(nullptr);
    EXPECT_EQ(json, "null");
}

// ============================================================================
// JSON Schema Compliance Tests
// ============================================================================

TEST_F(JSONSerializerTest, AllTokenFieldsArePresent) {
    Token token{TokenType::IDENTIFIER, "test", "file.c", 1, 1};
    std::string json = JSONSerializer::serializeToken(token);

    // Schema requires: type, value, line, column, filename
    EXPECT_TRUE(jsonContains(json, "\"type\""));
    EXPECT_TRUE(jsonContains(json, "\"value\""));
    EXPECT_TRUE(jsonContains(json, "\"line\""));
    EXPECT_TRUE(jsonContains(json, "\"column\""));
    EXPECT_TRUE(jsonContains(json, "\"filename\""));
}

TEST_F(JSONSerializerTest, AllExpressionFieldsArePresent) {
    auto expr = mycc_test::parse_expression_without_errors("x");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    // Schema requires: nodeType, line, column
    EXPECT_TRUE(jsonContains(json, "\"nodeType\""));
    EXPECT_TRUE(jsonContains(json, "\"line\""));
    EXPECT_TRUE(jsonContains(json, "\"column\""));
}

TEST_F(JSONSerializerTest, JSONObjectsAreWellFormed) {
    Token token{TokenType::IDENTIFIER, "x", "test.c", 1, 1};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');
    EXPECT_TRUE(isValidJSON(json));
}

// ============================================================================
// Complex AST Serialization Tests
// ============================================================================

TEST_F(JSONSerializerTest, SerializesComplexExpression) {
    auto expr = mycc_test::parse_expression_without_errors("(a + b) * (c - d)");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    EXPECT_TRUE(isValidJSON(json));
    // Should have multiple nested expressions
    EXPECT_GE(countOccurrences(json, "BinaryExpr"), 3);
}

TEST_F(JSONSerializerTest, SerializationPreservesStructure) {
    auto expr = mycc_test::parse_expression_without_errors("a + b");
    std::string json = JSONSerializer::serializeExpression(expr.get());

    // Root should be BinaryExpr with + operator
    EXPECT_TRUE(jsonContains(json, "\"operator\":\"+\""));
    // Should have left and right operands
    EXPECT_TRUE(jsonContains(json, "\"left\":{"));
    EXPECT_TRUE(jsonContains(json, "\"right\":{"));
}

// ============================================================================
// Edge Cases and Special Characters
// ============================================================================

TEST_F(JSONSerializerTest, HandlesSpecialCharactersInTokenValues) {
    Token token{TokenType::STRING_LITERAL, "Hello\nWorld", "test.c", 1, 1};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "\\n"));
}

TEST_F(JSONSerializerTest, HandlesUnicodeInIdentifiers) {
    // Note: This test assumes the lexer/parser can handle unicode
    Token token{TokenType::IDENTIFIER, "variablé", "test.c", 1, 1};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
}

TEST_F(JSONSerializerTest, HandlesEmptyStringValue) {
    Token token{TokenType::STRING_LITERAL, "", "test.c", 1, 1};
    std::string json = JSONSerializer::serializeToken(token);

    EXPECT_TRUE(isValidJSON(json));
    EXPECT_TRUE(jsonContains(json, "\"value\":\"\""));
}
