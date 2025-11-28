#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include "error_handler.h"
#include <memory>
#include <vector>
#include <string>

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

    // Program parsing (top-level entry point)
    std::vector<std::unique_ptr<Declaration>> parseProgram();

    // Declaration parsing
    std::unique_ptr<Declaration> parseDeclaration();
    std::unique_ptr<Declaration> parseVariableDeclaration();
    std::unique_ptr<Declaration> parseFunctionDeclaration();
    std::unique_ptr<Declaration> parseStructDefinition(); // USER STORY #19

    // Error handling public API
    ErrorHandler& getErrorHandler() { return error_handler_; }
    const ErrorHandler& getErrorHandler() const { return error_handler_; }
    bool hasErrors() const { return error_handler_.has_errors(); }

private:
    Lexer &lexer_;
    Token current_token_;
    ErrorHandler error_handler_; // Unified error reporting

    // Token management
    void advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string &error_message);

    // Error handling
    void reportError(const std::string &message);
    SourceLocation currentLocation() const;
    void synchronize(); // Skip to next statement boundary
    void synchronizeToDeclaration(); // Skip to next declaration

    // Operator precedence and associativity (User Story #3)
    int getOperatorPrecedence(TokenType type) const;
    bool isBinaryOperator(TokenType type) const;
    bool isUnaryOperator(TokenType type) const;
    std::string tokenTypeToOperatorString(TokenType type) const;

    // Helper methods
    bool isTypeKeyword(TokenType type) const;
    std::string parseType();
    std::vector<std::unique_ptr<ParameterDecl>> parseParameterList();

    // Declaration parsing helpers (refactored for clarity)
    std::unique_ptr<Declaration> parseStructDeclarationOrDefinition();
    std::vector<std::unique_ptr<VarDecl>> parseStructFieldList();
    std::unique_ptr<Declaration> parseFunctionDeclarationImpl(
        const Token& start_token,
        const std::string& type,
        const std::string& name,
        int pointerLevel
    );
    std::unique_ptr<Declaration> parseArrayDeclaration(
        const Token& start_token,
        const std::string& type,
        const std::string& name,
        int pointerLevel
    );
    std::unique_ptr<Declaration> parseVariableDeclarationImpl(
        const Token& start_token,
        const std::string& type,
        const std::string& name,
        int pointerLevel
    );
};

#endif // PARSER_H
