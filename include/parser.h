#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <memory>
#include <vector>

// Parser for C source code
// Converts tokens from the lexer into an Abstract Syntax Tree (AST)
class Parser
{
public:
    Parser(Lexer &lexer);

    // Main parsing methods
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parseBinaryExpression(int min_precedence = 0);

    // Specific expression parsers
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseLiteral();
    std::unique_ptr<Expression> parseParenthesizedExpression();

    // Statement parsing
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseIfStatement();
    std::unique_ptr<Statement> parseWhileStatement();
    std::unique_ptr<Statement> parseExpressionStatement();
    std::unique_ptr<Statement> parseCompoundStatement();

    // Declaration parsing
    std::unique_ptr<Declaration> parseDeclaration();
    std::unique_ptr<Declaration> parseVariableDeclaration();

private:
    Lexer &lexer_;
    Token current_token_;

    // Token management
    void advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &error_message);

    // Error handling
    void reportError(const std::string &message);
    SourceLocation currentLocation() const;

    // Operator precedence and associativity (User Story #3)
    int getOperatorPrecedence(TokenType type) const;
    bool isBinaryOperator(TokenType type) const;
    bool isUnaryOperator(TokenType type) const;
    std::string tokenTypeToOperatorString(TokenType type) const;

    // Helper methods
    bool isTypeKeyword(TokenType type) const;
    std::string parseType();
};

#endif // PARSER_H
