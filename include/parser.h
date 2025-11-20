#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <memory>
#include <vector>
#include <string>

// USER STORY #21: Error recovery support
struct ParseError
{
    std::string message;
    SourceLocation location;
    ParseError(const std::string &msg, const SourceLocation &loc)
        : message(msg), location(loc) {}
};

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
    std::unique_ptr<Statement> parseForStatement();
    std::unique_ptr<Statement> parseReturnStatement();
    std::unique_ptr<Statement> parseExpressionStatement();
    std::unique_ptr<Statement> parseCompoundStatement();

    // Declaration parsing
    std::unique_ptr<Declaration> parseDeclaration();
    std::unique_ptr<Declaration> parseVariableDeclaration();
    std::unique_ptr<Declaration> parseFunctionDeclaration();
    std::unique_ptr<Declaration> parseStructDefinition(); // USER STORY #19

    // Error handling public API (USER STORY #21)
    bool hadError() const { return !errors_.empty(); }
    const std::vector<ParseError> &getErrors() const { return errors_; }
    void clearErrors() { errors_.clear(); }

private:
    Lexer &lexer_;
    Token current_token_;

    // Token management
    void advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &error_message);

    // Error handling (USER STORY #21)
    void reportError(const std::string &message);
    SourceLocation currentLocation() const;
    void synchronize(); // Skip to next statement boundary
    void synchronizeToDeclaration(); // Skip to next declaration

    std::vector<ParseError> errors_; // Error collection

    // Operator precedence and associativity (User Story #3)
    int getOperatorPrecedence(TokenType type) const;
    bool isBinaryOperator(TokenType type) const;
    bool isUnaryOperator(TokenType type) const;
    std::string tokenTypeToOperatorString(TokenType type) const;

    // Helper methods
    bool isTypeKeyword(TokenType type) const;
    std::string parseType();
    std::vector<std::unique_ptr<ParameterDecl>> parseParameterList();
};

#endif // PARSER_H
