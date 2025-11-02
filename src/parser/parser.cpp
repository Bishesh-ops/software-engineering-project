#include "parser.h"

Parser::Parser(const std::vector<Token> &tokens)
    : tokens_(tokens), current_pos_(0) {}

Token Parser::current() const
{
    if (isAtEnd())
        return tokens_.back();
    return tokens_[current_pos_];
}

Token Parser::peek() const
{
    if (current_pos_ + 1 >= tokens_.size())
        return tokens_.back();
    return tokens_[current_pos_ + 1];
}

bool Parser::isAtEnd() const
{
    return current_pos_ >= tokens_.size() || current().type == TokenType::EOF_TOKEN;
}

Token Parser::advance()
{
    if (!isAtEnd())
        current_pos_++;
    return tokens_[current_pos_ - 1];
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd())
        return false;
    return current().type == type;
}

std::unique_ptr<ASTNode> Parser::parse()
{
    return nullptr;
}

