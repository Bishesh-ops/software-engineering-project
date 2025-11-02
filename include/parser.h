#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser;

std::unique_ptr<ASTNode> parseIdentifier(Parser &parser);

class Parser
{
public:
    Parser(const std::vector<Token> &tokens);
    std::unique_ptr<ASTNode> parse();

    Token current() const;
    Token peek() const;
    bool isAtEnd() const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type) const;

private:
    const std::vector<Token> &tokens_;
    size_t current_pos_;
};

#endif
