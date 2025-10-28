#include "lexer.h"
#include <cctype> // fro isalpha, is alpha num etc.

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

    //end of file
    if (c == '\0') {
        return Token(TokenType::EOF_TOKEN, "", start_line, start_column);
    }

    //a check for identifiers
    if (std::isalpha(c) || c == '_') {
        return scanIdentifierOrKeyword(start_line, start_column);
    }

    //check for character literals
    if (c == '\'') {
        return scanCharLiteral(start_line, start_column);
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

//lookup table for keywords
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"auto", TokenType::KW_AUTO},
    {"break", TokenType::KW_BREAK},
    {"case", TokenType::KW_CASE},
    {"char", TokenType::KW_CHAR},
    {"const", TokenType::KW_CONST},
    {"continue", TokenType::KW_CONTINUE},
    {"default", TokenType::KW_DEFAULT},
    {"do", TokenType::KW_DO},
    {"double", TokenType::KW_DOUBLE},
    {"else", TokenType::KW_ELSE},
    {"enum", TokenType::KW_ENUM},
    {"extern", TokenType::KW_EXTERN},
    {"float", TokenType::KW_FLOAT},
    {"for", TokenType::KW_FOR},
    {"goto", TokenType::KW_GOTO},
    {"if", TokenType::KW_IF},
    {"int", TokenType::KW_INT},
    {"long", TokenType::KW_LONG},
    {"register", TokenType::KW_REGISTER},
    {"return", TokenType::KW_RETURN},
    {"short", TokenType::KW_SHORT},
    {"signed", TokenType::KW_SIGNED},
    {"sizeof", TokenType::KW_SIZEOF},
    {"static", TokenType::KW_STATIC},
    {"struct", TokenType::KW_STRUCT},
    {"switch", TokenType::KW_SWITCH},
    {"typedef", TokenType::KW_TYPEDEF},
    {"union", TokenType::KW_UNION},
    {"unsigned", TokenType::KW_UNSIGNED},
    {"void", TokenType::KW_VOID},
    {"volatile", TokenType::KW_VOLATILE},
    {"while", TokenType::KW_WHILE}
};

//this checks if a given identifier is a keyword or not
TokenType Lexer::checkKeyword(const std::string &value) const
{
    auto it = keywords_.find(value);
    if (it != keywords_.end()) {
        return it->second; //returns the TokenType corresponding to the keyword
    }
    return TokenType::IDENTIFIER; //not a keyword, return IDENTIFIER
}

//reads an identifier and checks if it is a keyword
Token Lexer::scanIdentifierOrKeyword(int start_line, int start_column)
{
    std::string text;

    //first character is already known to be alphabetic or underscore
    text += peek();
    advance();

    while (std::isalnum(peek()) || peek() == '_') {
        text += peek();
        advance();
    }

    TokenType type = checkKeyword(text);
    return Token(type, text, start_line, start_column);   
}

//reads a character literal with escape sequence support
Token Lexer::scanCharLiteral(int start_line, int start_column)
{
    advance(); //consume opening '

    //empty literal: ''
    if (peek() == '\'') {
        advance();
        return Token(TokenType::UNKNOWN, "''", start_line, start_column);
    }

    //unterminated at newline or EOF
    if (peek() == '\n' || peek() == '\0') {
        return Token(TokenType::UNKNOWN, "'", start_line, start_column);
    }

    char actual_char;

    //handle escape sequences
    if (peek() == '\\') {
        advance(); //consume backslash

        if (peek() == '\0') {
            return Token(TokenType::UNKNOWN, "'\\", start_line, start_column);
        }

        char escape_char = peek();
        advance();

        //map escape sequences to actual characters
        switch (escape_char) {
            case 'n': actual_char = '\n'; break;
            case 't': actual_char = '\t'; break;
            case 'r': actual_char = '\r'; break;
            case '\\': actual_char = '\\'; break;
            case '\'': actual_char = '\''; break;
            case '0': actual_char = '\0'; break;
            default:
                //invalid escape sequence
                return Token(TokenType::UNKNOWN, std::string("'\\") + escape_char, start_line, start_column);
        }
    } else {
        //regular character
        actual_char = peek();
        advance();
    }

    //check for closing quote
    if (peek() != '\'') {
        //multi-character or unterminated
        if (peek() == '\n' || peek() == '\0') {
            return Token(TokenType::UNKNOWN, std::string("'") + actual_char, start_line, start_column);
        }
        //multi-character literal - consume rest until ' or newline/EOF
        std::string error_text = std::string("'") + actual_char;
        while (peek() != '\'' && peek() != '\n' && peek() != '\0') {
            error_text += peek();
            advance();
        }
        if (peek() == '\'') {
            error_text += peek();
            advance();
        }
        return Token(TokenType::UNKNOWN, error_text, start_line, start_column);
    }

    advance(); //consume closing '

    //valid character literal - store the actual character as value
    return Token(TokenType::CHAR_LITERAL, std::string(1, actual_char), start_line, start_column);
}

