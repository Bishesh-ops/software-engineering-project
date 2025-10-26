#include "lexer.h"

Lexer::Lexer(const std::string &source)
    : source_(source), current_pos_(0), current_line_(1), current_column_(1)
{
}

char Lexer::peek() const
{
    if (current_pos_ >= source_.length()) {
        return '\0'; // End of file
    }
    return source_[current_pos_];
}


char Lexer::advance()
{
    if ( current_pos_ >= source_.length()) {
        return '\0';
    }
    char current_char = source_[current_pos_];
    current_pos_++;
    current_column_++;

    if (current_char == '\n') {
        current_line_++;
        current_column_ = 1; //reset column at new line
    }
    return current_char;
}

void Lexer::skipWhitespace()
{
    while (true) {
        char current_char = peek();
        //Only skips spaces and tabs
        //does not skip newlines to keep track of line numbers (they might be significant)
        if (current_char == ' ' || current_char == '\t') {
            advance();
        } else {
            break;
        }
    }
}

Token Lexer::getNextToken()
{
    skipWhitespace();

    // Skip newlines using a loop instead of recursion
    while (peek() == '\n') {
        advance();
        skipWhitespace();
    }

    int start_line = current_line_;
    int start_column = current_column_;

    char c = peek();

    if (c == '\0') {
        return Token(TokenType::EOF_TOKEN, "", start_line, start_column);
    }

    advance();
    return Token(TokenType::UNKNOWN, std::string(1, c), start_line, start_column);
}

std::vector<Token> Lexer::lexAll()
{
    std::vector<Token> tokens;
    while (true) {
        Token token = getNextToken();
        tokens.push_back(token);
        if (token.type == TokenType::EOF_TOKEN) {
            break;
        }
    }
    return tokens;
}