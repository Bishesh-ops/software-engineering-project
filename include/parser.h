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

    // Specific expression parsers
    std::unique_ptr<Expression> parseIdentifier();
    std::unique_ptr<Expression> parseLiteral();

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
};

#endif // PARSER_H
