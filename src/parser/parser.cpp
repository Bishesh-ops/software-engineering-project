#include "parser.h"

Parser::Parser(const std::vector<Token> &tokens)
    : tokens_(tokens), current_pos_(0) {}

Token Parser::current() const
{
    if (current_pos_ >= tokens_.size())
    {
        if (!tokens_.empty())
            return tokens_.back();
        static const std::string empty_filename = "";
        static const std::string_view empty_value = "";
        static Token eof_token(TokenType::EOF_TOKEN, empty_value, empty_filename, 0, 0);
        return eof_token;
    }
    return tokens_[current_pos_];
}

Token Parser::peek() const
{
    if (current_pos_ + 1 >= tokens_.size())
    {
        if (!tokens_.empty())
            return tokens_.back();
        static const std::string empty_filename = "";
        static const std::string_view empty_value = "";
        static Token eof_token(TokenType::EOF_TOKEN, empty_value, empty_filename, 0, 0);
        return eof_token;
    }
    return tokens_[current_pos_ + 1];
}

bool Parser::isAtEnd() const
{
    if (current_pos_ >= tokens_.size())
        return true;
    return tokens_[current_pos_].type == TokenType::EOF_TOKEN;
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

