#include "lexer.h"
#include <cctype> // for isalpha, isalnum, isdigit, isxdigit

Lexer::Lexer(const std::string &source)
    : source_(source), current_pos_(0), current_line_(1), current_column_(1)
{
}

char Lexer::peek() const
{
    if (current_pos_ >= source_.length())
    {
        return '\0'; // End of file
    }
    return source_[current_pos_];
}

char Lexer::advance()
{
    if (current_pos_ >= source_.length())
    {
        return '\0';
    }

    char current_char = source_[current_pos_];
    current_pos_++;
    current_column_++;

    if (current_char == '\n')
    {
        current_line_++;
        current_column_ = 1; // Reset column at new line
    }
    return current_char;
}

void Lexer::skipWhitespace()
{
    while (true)
    {
        char current_char = peek();
        // Skips spaces, tabs, AND carriage returns (\r)
        if (current_char == ' ' || current_char == '\t' || current_char == '\r')
        {
            advance();
        }
        else
        {
            break;
        }
    }
}

// Skips C-style comments (// and /* */)
void Lexer::skipComment()
{
    // Check for single-line comment //
    if (peek() == '/')
    {
        advance(); // Consume the second /
        while (peek() != '\n' && peek() != '\0')
        {
            advance(); // Consume characters until newline or EOF
        }
        return;
    }

    // Check for multi-line comment /* */
    if (peek() == '*')
    {
        advance(); // Consume the '*'
        while (true)
        {
            if (peek() == '\0')
            {
                // Unterminated multi-line comment
                return;
            }
            if (peek() == '*')
            {
                advance(); // Consume '*'
                if (peek() == '/')
                {
                    advance(); // Consume '/' - comment ended
                    return;
                }
            }
            else
            {
                advance(); // Consume any other character
            }
        }
    }
    // If we get here, it means the caller consumed a '/' but the next char
    // wasn't another '/' or a '*'. The caller should handle putting it back.
}

Token Lexer::getNextToken()
{
    // --- 1. Preparation: Skip whitespace, newlines, and comments ---
    while (true)
    { // Loop to handle multiple whitespace/comments in a row
        skipWhitespace();

        // Consume any standalone newlines
        while (peek() == '\n')
        {
            advance();
            skipWhitespace(); // Skip indentation on the new line
        }

        // Check for and skip comments
        if (peek() == '/')
        {
            advance(); // Consume the first '/'
            if (peek() == '/' || peek() == '*')
            {
                // It's a comment, skip it and restart the loop
                skipComment();
                continue; // Go back to skipWhitespace/newlines
            }
            else
            {
                // It's just a division operator, "put back" the '/'
                // by decrementing position and column
                current_pos_--;
                current_column_--;
                // Now break the loop and proceed to token classification for '/'
                break;
            }
        }
        else
        {
            // Not whitespace, newline, or comment start - ready for token
            break;
        }
    }

    // Store token start position *after* skipping whitespace/comments
    int start_line = current_line_;
    int start_column = current_column_;

    char c = peek();

    // --- 2. Check for EOF ---
    if (c == '\0')
    {
        return Token(TokenType::EOF_TOKEN, "", start_line, start_column);
    }

    // --- 3. Token Classification ---

    // Identifiers and Keywords: [a-zA-Z_][a-zA-Z0-9_]*
    if (std::isalpha(c) || c == '_')
    {
        return scanIdentifierOrKeyword(start_line, start_column);
    }

    // Number Literals: 123, 0.5, 0x1A, .123
    // Check for digit OR '.' followed by a digit
    if (std::isdigit(c) || (c == '.' && current_pos_ + 1 < source_.length() && std::isdigit(source_[current_pos_ + 1])))
    {
        return scanNumber(start_line, start_column);
    }

    // Character Literals: 'a'
    if (c == '\'')
    {
        return scanCharLiteral(start_line, start_column);
    }

    // String Literals: "hello"
    if (c == '"')
    {
        return scanStringLiteral(start_line, start_column);
    }

    // Preprocessor Symbols: #, ##
    if (c == '#')
    {
        advance(); // Consume '#'
        if (peek() == '#')
        {
            advance(); // Consume second '#'
            return Token(TokenType::DOUBLE_HASH, "##", start_line, start_column);
        }
        return Token(TokenType::HASH, "#", start_line, start_column);
    }

    // Operators (Includes '/' which is only reached if not a comment)
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
        c == '<' || c == '>' || c == '!' || c == '~' || c == '&' ||
        c == '|' || c == '^' || c == '=' || c == '?' || c == ':')
    {
        return scanOperator(start_line, start_column);
    }

    // Delimiters
    if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' ||
        c == ']' || c == ';' || c == ',' || c == '.')
    {
        return scanDelimiter(start_line, start_column);
    }

    // --- 4. Handle Unrecognized Characters ---
    advance();
    return Token(TokenType::UNKNOWN, std::string(1, c), start_line, start_column);
}

std::vector<Token> Lexer::lexAll()
{
    std::vector<Token> tokens;
    while (true)
    {
        Token token = getNextToken();
        tokens.push_back(token);
        if (token.type == TokenType::EOF_TOKEN)
        {
            break;
        }
    }
    return tokens;
}

// Handles integers (dec, hex, oct), floats (., e/E), and suffixes (L, U, f).
Token Lexer::scanNumber(int start_line, int start_column)
{
    std::string text;
    TokenType type = TokenType::INT_LITERAL; // Assume integer unless proven otherwise
    bool is_hex = false;

    // --- 1. Handle Integer/Prefix Part ---
    if (peek() == '0')
    {
        text += advance(); // Consume '0'

        if (peek() == 'x' || peek() == 'X')
        {
            // Hexadecimal: 0x...
            is_hex = true;
            text += advance(); // Consume 'x' or 'X'
            while (std::isxdigit(peek()))
            {
                text += advance();
            }
        }
        else
        {
            // Octal/Decimal '0': 0... (could be 0, 0123, 0.5)
            while (std::isdigit(peek()))
            {
                text += advance();
            }
        }
    }
    else if (std::isdigit(peek()))
    {
        // Decimal: [1-9]...
        while (std::isdigit(peek()))
        {
            text += advance();
        }
    }
    else if (peek() == '.')
    {
        // Float starting with '.': .123
        type = TokenType::FLOAT_LITERAL;
        text += advance(); // Consume '.'
        while (std::isdigit(peek()))
        {
            text += advance();
        }
    }

    // --- 2. Handle Floating Point Parts (if not hex) ---
    if (!is_hex)
    {
        // Check for Fractional Part: 123.45
        if (peek() == '.')
        {
            if (type == TokenType::INT_LITERAL)
            {
                // This is the first time we've seen a '.', promote to float
                type = TokenType::FLOAT_LITERAL;
                text += advance(); // Consume '.'
                while (std::isdigit(peek()))
                {
                    text += advance();
                }
            }
        }

        // Check for Exponent Part: e/E
        char exp_char = peek();
        if (exp_char == 'e' || exp_char == 'E')
        {
            type = TokenType::FLOAT_LITERAL;
            text += advance(); // Consume 'e' or 'E'

            if (peek() == '+' || peek() == '-')
            {
                text += advance();
            }
            while (std::isdigit(peek()))
            {
                text += advance();
            }
        }
    }

    // --- 3. Handle Suffixes ---
    if (type == TokenType::FLOAT_LITERAL)
    {
        // Float suffixes (f/F for float, l/L for long double)
        char suffix_f = peek();
        if (suffix_f == 'f' || suffix_f == 'F' || suffix_f == 'l' || suffix_f == 'L')
        {
            text += advance();
        }
        return Token(type, text, start_line, start_column);
    }
    else
    {
        // Integer suffixes (U, L, LL) - order can be U, L, LL, UL, LU, etc.
        bool has_u = false;
        bool has_l = false;

        // Greedily consume suffixes
        if (peek() == 'u' || peek() == 'U')
        {
            text += advance();
            has_u = true;
        }

        if (peek() == 'l' || peek() == 'L')
        {
            text += advance();
            if (peek() == 'l' || peek() == 'L')
            {
                text += advance(); // Long Long
            }
            has_l = true;
        }

        // Check for 'U' again in case of "LU" order
        if (!has_u && (peek() == 'u' || peek() == 'U'))
        {
            text += advance();
        }

        // Check for 'L' again in case of "UL" order
        if (!has_l && (peek() == 'l' || peek() == 'L'))
        {
            text += advance();
            if (peek() == 'l' || peek() == 'L')
            {
                text += advance(); // Long Long
            }
        }

        return Token(type, text, start_line, start_column);
    }
}

// Static map of C keywords.
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
    {"while", TokenType::KW_WHILE}};

TokenType Lexer::checkKeyword(const std::string &value) const
{
    auto it = keywords_.find(value);
    if (it != keywords_.end())
    {
        return it->second; // It's a keyword
    }
    return TokenType::IDENTIFIER; // Not a keyword
}

Token Lexer::scanIdentifierOrKeyword(int start_line, int start_column)
{
    std::string text;
    text += advance(); // First character is already checked

    // Consume subsequent alphanumeric or underscore characters
    while (std::isalnum(peek()) || peek() == '_')
    {
        text += advance();
    }

    // Check if the identifier is actually a keyword
    TokenType type = checkKeyword(text);
    return Token(type, text, start_line, start_column);
}

// Handles: 'a', '\n', and errors: '', 'abc', '\z'
Token Lexer::scanCharLiteral(int start_line, int start_column)
{
    advance(); // Consume opening '

    // Handle empty literal: ''
    if (peek() == '\'')
    {
        advance();
        return Token(TokenType::UNKNOWN, "''", start_line, start_column);
    }

    // Handle unterminated at newline or EOF
    if (peek() == '\n' || peek() == '\0')
    {
        return Token(TokenType::UNKNOWN, "'", start_line, start_column);
    }

    char actual_char;

    // Handle escape sequences
    if (peek() == '\\')
    {
        advance(); // Consume backslash

        if (peek() == '\0')
        {
            return Token(TokenType::UNKNOWN, "'\\", start_line, start_column);
        }

        char escape_char = advance(); // Consume escape character

        // Map C escape sequences
        switch (escape_char)
        {
        case 'n':
            actual_char = '\n';
            break;
        case 't':
            actual_char = '\t';
            break;
        case 'r':
            actual_char = '\r';
            break;
        case '\\':
            actual_char = '\\';
            break;
        case '\'':
            actual_char = '\'';
            break;
        case '0':
            actual_char = '\0';
            break;
        // TODO: Add other escapes like \b, \f, \v, \xNN, \NNN
        default:
            // Invalid escape sequence
            return Token(TokenType::UNKNOWN, std::string("'\\") + escape_char, start_line, start_column);
        }
    }
    else
    {
        // Regular character
        actual_char = advance();
    }

    // Check for closing quote
    if (peek() != '\'')
    {
        // Error: multi-character or unterminated
        std::string error_text = std::string("'") + actual_char;
        while (peek() != '\'' && peek() != '\n' && peek() != '\0')
        {
            error_text += advance();
        }
        if (peek() == '\'')
        {
            error_text += advance(); // Consume closing '
        }
        return Token(TokenType::UNKNOWN, error_text, start_line, start_column);
    }

    advance(); // Consume closing '

    // Valid char literal. Store the *actual* character value.
    return Token(TokenType::CHAR_LITERAL, std::string(1, actual_char), start_line, start_column);
}

// Handles: "hello", "line1\n", and unterminated "hello
Token Lexer::scanStringLiteral(int start_line, int start_column)
{
    advance(); // Consume opening "

    std::string processed_value; // The actual string value (escapes processed)

    while (peek() != '"')
    {
        // Check for unterminated string
        if (peek() == '\n' || peek() == '\0')
        {
            return Token(TokenType::UNKNOWN,
                         std::string("\"") + processed_value,
                         start_line, start_column);
        }

        // Handle escape sequences
        if (peek() == '\\')
        {
            advance(); // Consume '\'
            if (peek() == '\0')
            { // Unterminated escape
                return Token(TokenType::UNKNOWN,
                             std::string("\"") + processed_value + "\\",
                             start_line, start_column);
            }
            char escape_char = advance(); // Consume escape character

            // Map C escape sequences
            switch (escape_char)
            {
            case 'n':
                processed_value += '\n';
                break;
            case 't':
                processed_value += '\t';
                break;
            case 'r':
                processed_value += '\r';
                break;
            case 'b':
                processed_value += '\b';
                break;
            case 'f':
                processed_value += '\f';
                break;
            case 'v':
                processed_value += '\v';
                break;
            case '0':
                processed_value += '\0';
                break;
            case '\\':
                processed_value += '\\';
                break;
            case '"':
                processed_value += '"';
                break;
            case '\'':
                processed_value += '\'';
                break;
            // TODO: Add hex (\xNN) and octal (\NNN) escapes
            default:
                // Treat invalid escapes as literal characters
                processed_value += escape_char;
                break;
            }
        }
        // Handle regular character
        else
        {
            processed_value += advance();
        }
    }
    advance(); // Consume closing "

    return Token(TokenType::STRING_LITERAL, processed_value, start_line, start_column);
}

// Uses lookahead (peek()) to handle multi-character operators
Token Lexer::scanOperator(int start_line, int start_column)
{
    char c = advance(); // Consume the first character

    // Use a switch on the *first* char to decide.
    switch (c)
    {
    case '+':
        if (peek() == '+')
        {
            advance();
            return Token(TokenType::OP_INC, "++", start_line, start_column);
        }
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_PLUS_ASSIGN, "+=", start_line, start_column);
        }
        return Token(TokenType::OP_PLUS, "+", start_line, start_column);

    case '-':
        if (peek() == '-')
        {
            advance();
            return Token(TokenType::OP_DEC, "--", start_line, start_column);
        }
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_MINUS_ASSIGN, "-=", start_line, start_column);
        }
        if (peek() == '>')
        {
            advance();
            return Token(TokenType::ARROW, "->", start_line, start_column);
        }
        return Token(TokenType::OP_MINUS, "-", start_line, start_column);

    case '*':
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_STAR_ASSIGN, "*=", start_line, start_column);
        }
        return Token(TokenType::OP_STAR, "*", start_line, start_column);

    case '/':
        // We only reach here if it wasn't a comment (handled in getNextToken)
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_SLASH_ASSIGN, "/=", start_line, start_column);
        }
        return Token(TokenType::OP_SLASH, "/", start_line, start_column);

    case '%':
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_MOD_ASSIGN, "%=", start_line, start_column);
        }
        return Token(TokenType::OP_MOD, "%", start_line, start_column);

    case '<':
        if (peek() == '<')
        {
            advance(); // Consume '<'
            if (peek() == '=')
            {
                advance(); // Consume '='
                return Token(TokenType::OP_LSHIFT_ASSIGN, "<<=", start_line, start_column);
            }
            return Token(TokenType::OP_LSHIFT, "<<", start_line, start_column);
        }
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_LE, "<=", start_line, start_column);
        }
        return Token(TokenType::OP_LT, "<", start_line, start_column);

    case '>':
        if (peek() == '>')
        {
            advance(); // Consume '>'
            if (peek() == '=')
            {
                advance(); // Consume '='
                return Token(TokenType::OP_RSHIFT_ASSIGN, ">>=", start_line, start_column);
            }
            return Token(TokenType::OP_RSHIFT, ">>", start_line, start_column);
        }
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_GE, ">=", start_line, start_column);
        }
        return Token(TokenType::OP_GT, ">", start_line, start_column);

    case '=':
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_EQ, "==", start_line, start_column);
        }
        return Token(TokenType::OP_ASSIGN, "=", start_line, start_column);

    case '!':
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_NE, "!=", start_line, start_column);
        }
        return Token(TokenType::OP_NOT, "!", start_line, start_column);

    case '&':
        if (peek() == '&')
        {
            advance();
            return Token(TokenType::OP_AND, "&&", start_line, start_column);
        }
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_AND_ASSIGN, "&=", start_line, start_column);
        }
        return Token(TokenType::OP_BIT_AND, "&", start_line, start_column);

    case '|':
        if (peek() == '|')
        {
            advance();
            return Token(TokenType::OP_OR, "||", start_line, start_column);
        }
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_OR_ASSIGN, "|=", start_line, start_column);
        }
        return Token(TokenType::OP_BIT_OR, "|", start_line, start_column);

    case '^':
        if (peek() == '=')
        {
            advance();
            return Token(TokenType::OP_XOR_ASSIGN, "^=", start_line, start_column);
        }
        return Token(TokenType::OP_BIT_XOR, "^", start_line, start_column);

    case '~':
        return Token(TokenType::OP_BIT_NOT, "~", start_line, start_column);

    case '?':
        return Token(TokenType::OP_QUESTION, "?", start_line, start_column);

    case ':':
        return Token(TokenType::COLON, ":", start_line, start_column);

    default:
        // This case should be unreachable
        return Token(TokenType::UNKNOWN, std::string(1, c), start_line, start_column);
    }
}

// Scans a simple, single-character delimiter.
Token Lexer::scanDelimiter(int start_line, int start_column)
{
    char c = advance(); // Consume the delimiter

    switch (c)
    {
    case '(':
        return Token(TokenType::LPAREN, "(", start_line, start_column);
    case ')':
        return Token(TokenType::RPAREN, ")", start_line, start_column);
    case '{':
        return Token(TokenType::LBRACE, "{", start_line, start_column);
    case '}':
        return Token(TokenType::RBRACE, "}", start_line, start_column);
    case '[':
        return Token(TokenType::LBRACKET, "[", start_line, start_column);
    case ']':
        return Token(TokenType::RBRACKET, "]", start_line, start_column);
    case ';':
        return Token(TokenType::SEMICOLON, ";", start_line, start_column);
    case ',':
        return Token(TokenType::COMMA, ",", start_line, start_column);
    case '.':
        return Token(TokenType::DOT, ".", start_line, start_column);

    default:
        // This case should be unreachable
        return Token(TokenType::UNKNOWN, std::string(1, c), start_line, start_column);
    }
}