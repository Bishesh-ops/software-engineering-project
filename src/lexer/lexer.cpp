#include "lexer.h"
#include <cctype> // for isalpha, is alpha num, isdigit, isxdigit

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
    
    // Check for integer literals (decimal, octal, hex)
    if (std::isdigit(c)) {
        return scanNumber(start_line, start_column);
    }

    // Currently only handles simple single-character tokens and errors here.
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

// Scans for all number literals (integer, hex, octal, float)
Token Lexer::scanNumber(int start_line, int start_column)
{
    std::string text;
    TokenType type = TokenType::INT_LITERAL; // Assume integer unless proven otherwise
    bool is_hex = false; // Flag to track hex literals, which cannot be floats

    // 1. Scan the integer prefix or initial dot
    if (peek() == '0') {
        text += advance(); // Consume '0'
        
        if (peek() == 'x' || peek() == 'X') {
            // Hexadecimal Integer: Set flag and consume hex digits
            is_hex = true;
            text += advance(); // Consume 'x' or 'X'
            
            while (std::isxdigit(peek())) {
                text += advance();
            }
            
        } else {
            // Octal/Decimal prefix '0' - consume digits
            while (std::isdigit(peek())) {
                text += advance();
            }
        }
    } else if (std::isdigit(peek())) {
        // Decimal Integer (starts with 1-9) - consume digits
        while (std::isdigit(peek())) {
            text += advance();
        }
    } else if (peek() == '.') {
        // Float starting with '.' (e.g. .123)
        type = TokenType::FLOAT_LITERAL;
        text += advance(); // Consume '.'
        
        // Consume fractional digits
        while (std::isdigit(peek())) {
            text += advance();
        }
    }

    // 2. Check for Floating Point elements only if it's not a hexadecimal literal
    if (!is_hex) {
        
        // Check for Fractional Part (for numbers that started with digits, e.g., 123.45)
        if (peek() == '.') {
            type = TokenType::FLOAT_LITERAL;
            text += advance(); // Consume '.'
            
            // Consume digits after the decimal point
            while (std::isdigit(peek())) {
                text += advance();
            }
        }

        // Check for Exponent Part
        char exp_char = peek();
        if (exp_char == 'e' || exp_char == 'E') {
            type = TokenType::FLOAT_LITERAL;
            text += advance(); // Consume 'e' or 'E'
            
            // Consume optional sign (+/-)
            if (peek() == '+' || peek() == '-') {
                text += advance();
            }
            
            // Consume exponent digits (mandatory for a valid C float exponent)
            while (std::isdigit(peek())) {
                text += advance();
            }
        }
    }

    // 3. Handle Suffixes based on detected type
    if (type == TokenType::FLOAT_LITERAL) {
        // Consume float suffixes (f/F for float, l/L for long double)
        char suffix_f = peek();
        if (suffix_f == 'f' || suffix_f == 'F' || suffix_f == 'l' || suffix_f == 'L') {
            text += advance();
        }
        return Token(type, text, start_line, start_column);

    } else {
        // Integer (Decimal/Octal/Hex) suffixes
        
        // Consume integer suffixes: L, U, LL (or l, u, ll) - order can vary.
        char c_suffix = peek();
        
        // Consume optional 'U' or 'u' first
        if (c_suffix == 'u' || c_suffix == 'U') {
            text += advance();
            c_suffix = peek();
        }
        
        // Consume optional 'L' or 'LL' ('l' or 'll')
        if (c_suffix == 'l' || c_suffix == 'L') {
            text += advance();
            if (peek() == c_suffix) {
                text += advance(); // Consume second 'l' or 'L' for long long
            }
            c_suffix = peek();
        }
        
        // Consume optional 'U' or 'u' again (handles LU order)
        if (c_suffix == 'u' || c_suffix == 'U') {
            text += advance();
        }
        
        return Token(type, text, start_line, start_column);
    }
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