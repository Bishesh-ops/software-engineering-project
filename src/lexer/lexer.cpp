#include "lexer.h"
#include <cctype> // for isalpha, isalnum, isdigit, isxdigit

// Initialize error_count_ in the constructor
Lexer::Lexer(const std::string &source)
    : source_(source), current_pos_(0), current_line_(1), current_column_(1), error_count_(0) // Initialize error_count_
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
}

Token Lexer::getNextToken()
{
    // --- Check error limit ---
    if (error_count_ >= MAX_ERRORS)
    {
        // Return EOF early if too many errors occurred
        return Token(TokenType::EOF_TOKEN, "", current_line_, current_column_);
    }

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
                current_pos_--;
                current_column_--;
                break;
            }
        }
        else
        {
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
    Token result_token = Token(TokenType::UNKNOWN, "", start_line, start_column); // Default to UNKNOWN

    // Identifiers and Keywords
    if (std::isalpha(c) || c == '_')
    {
        result_token = scanIdentifierOrKeyword(start_line, start_column);
    }
    // Number Literals
    else if (std::isdigit(c) || (c == '.' && current_pos_ + 1 < source_.length() && std::isdigit(source_[current_pos_ + 1])))
    {
        result_token = scanNumber(start_line, start_column);
    }
    // Character Literals
    else if (c == '\'')
    {
        result_token = scanCharLiteral(start_line, start_column);
    }
    // String Literals
    else if (c == '"')
    {
        result_token = scanStringLiteral(start_line, start_column);
    }
    // Preprocessor Symbols
    else if (c == '#')
    {
        advance(); // Consume '#'
        if (peek() == '#')
        {
            advance(); // Consume second '#'
            result_token = Token(TokenType::DOUBLE_HASH, "##", start_line, start_column);
        }
        else
        {
            result_token = Token(TokenType::HASH, "#", start_line, start_column);
        }
    }
    // Operators
    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
             c == '<' || c == '>' || c == '!' || c == '~' || c == '&' ||
             c == '|' || c == '^' || c == '=' || c == '?' || c == ':')
    {
        result_token = scanOperator(start_line, start_column);
    }
    // Delimiters
    else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' ||
             c == ']' || c == ';' || c == ',' || c == '.')
    {
        result_token = scanDelimiter(start_line, start_column);
    }
    // --- 4. Handle Unrecognized Characters ---
    else
    {
        advance();
        result_token = Token(TokenType::UNKNOWN, std::string(1, c), start_line, start_column);
    }

    // --- Increment error count if token is UNKNOWN ---
    if (result_token.type == TokenType::UNKNOWN)
    {
        error_count_++;
    }

    return result_token;
}

// Handles integers (dec, hex, oct), floats (., e/E), and suffixes (L, U, f).
// Returns UNKNOWN on invalid format after valid prefix.
Token Lexer::scanNumber(int start_line, int start_column)
{
    std::string text;
    TokenType type = TokenType::INT_LITERAL;
    bool is_hex = false;
    // Keep track of the start to return the full invalid text if needed
    size_t start_pos = current_pos_;

    // --- 1. Handle Integer/Prefix Part ---
    if (peek() == '0')
    {
        text += advance(); // Consume '0'
        if (peek() == 'x' || peek() == 'X')
        {
            is_hex = true;
            text += advance(); // Consume 'x' or 'X'
            // Must have at least one hex digit after 0x
            if (!std::isxdigit(peek()))
            {
                return Token(TokenType::UNKNOWN, text, start_line, start_column);
            }
            while (std::isxdigit(peek()))
            {
                text += advance();
            }
        }
        else
        { // Could be octal or start of float 0.x
            while (std::isdigit(peek()))
            {
                // In C, octal literals cannot contain 8 or 9
                if (peek() >= '8')
                {
                    // It's likely a decimal starting with 0 or a float like 0.8
                    // We'll let the final check catch potential errors like 08a
                    break; // Stop consuming octal-like digits
                }
                text += advance();
            }
        }
    }
    else if (std::isdigit(peek()))
    { // Decimal
        while (std::isdigit(peek()))
        {
            text += advance();
        }
    }
    else if (peek() == '.')
    { // Float starting with '.'
        type = TokenType::FLOAT_LITERAL;
        text += advance(); // Consume '.'
                           // Must have at least one digit after starting '.'
        if (!std::isdigit(peek()))
        {
            return Token(TokenType::UNKNOWN, text, start_line, start_column);
        }
        while (std::isdigit(peek()))
        {
            text += advance();
        }
    }
    else
    {
        // Should not happen if called correctly from getNextToken
        return Token(TokenType::UNKNOWN, text, start_line, start_column);
    }

    // --- 2. Handle Floating Point Parts (if not hex) ---
    if (!is_hex)
    {
        bool had_decimal = (type == TokenType::FLOAT_LITERAL); // Track if we already saw '.' starting the number
        // Check for Fractional Part: 123.45 or 123.
        if (peek() == '.')
        {
            if (!had_decimal) // Only allow one '.'
            {
                type = TokenType::FLOAT_LITERAL;
                text += advance();           // Consume '.'
                while (std::isdigit(peek())) // Consume optional digits after '.'
                {
                    text += advance();
                }
                had_decimal = true; // Mark that we've now seen a decimal point
            }
            else
            {
                // Already had a '.', seeing another is invalid (e.g., .123.)
                // Let the check at the end handle this
            }
        }

        // Check for Exponent Part: e/E
        char exp_char = peek();
        if (exp_char == 'e' || exp_char == 'E')
        {
            // Exponent implies float, cannot be hex
            char prev_char = text.empty() ? '\0' : text.back();
            // Exponent must follow a digit or a decimal point
            if (std::isdigit(prev_char) || prev_char == '.')
            {
                type = TokenType::FLOAT_LITERAL; // Ensure type is float
                text += advance();               // Consume 'e' or 'E'

                // Optional sign
                if (peek() == '+' || peek() == '-')
                {
                    text += advance();
                }
                // Exponent digits are mandatory after e/E[+/-]
                if (!std::isdigit(peek()))
                {
                    // Error: Malformed exponent (e.g., "1e", "1e+")
                    while (std::isalnum(peek()))
                    { // Consume potential invalid part
                        text += advance();
                    }
                    // Return the full consumed text as UNKNOWN
                    std::string full_text = source_.substr(start_pos, current_pos_ - start_pos);
                    return Token(TokenType::UNKNOWN, full_text, start_line, start_column);
                }
                while (std::isdigit(peek()))
                {
                    text += advance();
                }
            }
            // else: 'e'/'E' not after digit/. is invalid, let end check handle it (e.g., 0xe+10)
        }
    }

    // --- 3. Handle Suffixes ---
    // Keep track of suffixes seen to check validity later
    std::string suffix_str;
    if (type == TokenType::FLOAT_LITERAL)
    {
        // Float suffixes (f/F for float, l/L for long double)
        char s = peek();
        if (!is_hex && (s == 'f' || s == 'F' || s == 'l' || s == 'L'))
        {
            suffix_str += advance();
        }
    }
    else // Integer suffixes (can apply to hex/octal/decimal)
    {
        bool has_u = false;
        bool has_l = false;
        bool has_ll = false;

        // Try to consume up to 3 suffix chars (U, L, LL in any order)
        for (int i = 0; i < 3; ++i)
        {
            char s = peek();
            if (!has_u && (s == 'u' || s == 'U'))
            {
                has_u = true;
                suffix_str += advance();
            }
            else if (!has_ll && (s == 'l' || s == 'L'))
            {
                if (has_l)
                { // This is the second L
                    has_ll = true;
                }
                has_l = true;
                suffix_str += advance();
            }
            else
            {
                break; // Not a valid integer suffix character
            }
        }
    }
    // Append consumed suffixes to the main text
    text += suffix_str;

    // --- FINAL CHECK: Check for invalid characters immediately following the number/suffix ---
    if (std::isalnum(peek()) || peek() == '.')
    { // Check for letters, numbers, or unexpected '.'
        // Consume the rest of the invalid sequence
        while (std::isalnum(peek()) || peek() == '.')
        {
            text += advance();
        }
        // It's an invalid number format (e.g., "123a", "0xG", "1.2.3", "0f", "1.f", "10ULX")
        std::string full_text = source_.substr(start_pos, current_pos_ - start_pos);
        return Token(TokenType::UNKNOWN, full_text, start_line, start_column);
    }

    // If we passed all checks, it's a valid number
    std::string final_value = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(type, final_value, start_line, start_column);
}

// Static map of C keywords.
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"auto", TokenType::KW_AUTO}, {"break", TokenType::KW_BREAK}, {"case", TokenType::KW_CASE}, {"char", TokenType::KW_CHAR}, {"const", TokenType::KW_CONST}, {"continue", TokenType::KW_CONTINUE}, {"default", TokenType::KW_DEFAULT}, {"do", TokenType::KW_DO}, {"double", TokenType::KW_DOUBLE}, {"else", TokenType::KW_ELSE}, {"enum", TokenType::KW_ENUM}, {"extern", TokenType::KW_EXTERN}, {"float", TokenType::KW_FLOAT}, {"for", TokenType::KW_FOR}, {"goto", TokenType::KW_GOTO}, {"if", TokenType::KW_IF}, {"int", TokenType::KW_INT}, {"long", TokenType::KW_LONG}, {"register", TokenType::KW_REGISTER}, {"return", TokenType::KW_RETURN}, {"short", TokenType::KW_SHORT}, {"signed", TokenType::KW_SIGNED}, {"sizeof", TokenType::KW_SIZEOF}, {"static", TokenType::KW_STATIC}, {"struct", TokenType::KW_STRUCT}, {"switch", TokenType::KW_SWITCH}, {"typedef", TokenType::KW_TYPEDEF}, {"union", TokenType::KW_UNION}, {"unsigned", TokenType::KW_UNSIGNED}, {"void", TokenType::KW_VOID}, {"volatile", TokenType::KW_VOLATILE}, {"while", TokenType::KW_WHILE}};

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
    size_t start_pos = current_pos_; // Remember start position
    text += advance();               // First character is already checked

    // Consume subsequent alphanumeric or underscore characters
    while (std::isalnum(peek()) || peek() == '_')
    {
        text += advance();
    }
    std::string final_value = source_.substr(start_pos, current_pos_ - start_pos); // Get full value
    // Check if the identifier is actually a keyword
    TokenType type = checkKeyword(final_value);
    return Token(type, final_value, start_line, start_column);
}

// In src/lexer/lexer.cpp

// Handles: 'a', '\n', and errors: '', 'abc', '\z'
Token Lexer::scanCharLiteral(int start_line, int start_column)
{
    size_t start_pos = current_pos_; // Remember start for raw value
    advance();                       // Consume opening '

    if (peek() == '\'')
    { // Empty ''
        advance();
        return Token(TokenType::UNKNOWN, "''", start_line, start_column);
    }
    if (peek() == '\n' || peek() == '\0')
    { // Unterminated '
        // Don't advance, report error with just the opening quote
        return Token(TokenType::UNKNOWN, "'", start_line, start_column);
    }

    // char actual_char; // REMOVED - Was unused

    // Handle escape sequences
    if (peek() == '\\')
    {
        advance(); // Consume backslash
        if (peek() == '\0')
        {
            // Unterminated escape sequence
            std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, raw_error, start_line, start_column);
        }
        char escape_char = advance(); // Consume escape character
        switch (escape_char)
        {
        // Check for *valid* C character escapes
        case 'n':
        case 't':
        case 'r':
        case '\\':
        case '\'':
        case '0':
            // TODO: Add other valid escapes like \b, \f, \v...
            break; // Valid escape, proceed
        default:
            // Invalid escape sequence
            std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, raw_error, start_line, start_column);
        }
    }
    else
    {              // Regular character
        advance(); // Just consume the character
    }

    if (peek() != '\'')
    { // Multi-character or unterminated after char
        std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
        // Consume until closing quote or newline/EOF
        while (peek() != '\'' && peek() != '\n' && peek() != '\0')
        {
            raw_error += advance();
        }
        if (peek() == '\'')
        {
            raw_error += advance();
        } // Include closing quote in error if found
        return Token(TokenType::UNKNOWN, raw_error, start_line, start_column);
    }

    advance();                                                                   // Consume closing '
    std::string raw_value = source_.substr(start_pos, current_pos_ - start_pos); // Get the full raw 'x' or '\n' string
    return Token(TokenType::CHAR_LITERAL, raw_value, start_line, start_column);
}

// Handles: "hello", "line1\n", and unterminated "hello
Token Lexer::scanStringLiteral(int start_line, int start_column)
{
    size_t start_pos = current_pos_; // Remember start for raw value
    advance();                       // Consume opening "

    while (peek() != '"')
    {
        if (peek() == '\n' || peek() == '\0')
        { // Unterminated string
            std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, raw_error, start_line, start_column);
        }

        if (peek() == '\\')
        {              // Handle escape sequences
            advance(); // Consume '\'
            if (peek() == '\0')
            { // Unterminated escape
                std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
                return Token(TokenType::UNKNOWN, raw_error, start_line, start_column);
            }
            // char escape_char = advance(); // REMOVED - Was unused
            advance(); // Just consume the character after backslash
        }
        else
        { // Handle regular character
            advance();
        }
    }
    advance();                                                                   // Consume closing "
    std::string raw_value = source_.substr(start_pos, current_pos_ - start_pos); // Get the full raw "..." string
    return Token(TokenType::STRING_LITERAL, raw_value, start_line, start_column);
}

// Uses lookahead (peek()) to handle multi-character operators
Token Lexer::scanOperator(int start_line, int start_column)
{
    size_t start_pos = current_pos_; // Remember start position
    char c = advance();              // Consume the first character

    // Use a switch on the *first* char to decide.
    switch (c)
    {
    case '+':
        if (peek() == '+')
        {
            advance();
        }
        else if (peek() == '=')
        {
            advance();
        }
        break; // Break after consuming potential second/third char
    case '-':
        if (peek() == '-')
        {
            advance();
        }
        else if (peek() == '=')
        {
            advance();
        }
        else if (peek() == '>')
        {
            advance();
        }
        break;
    case '*':
        if (peek() == '=')
        {
            advance();
        }
        break;
    case '/':
        if (peek() == '=')
        {
            advance();
        }
        break;
    case '%':
        if (peek() == '=')
        {
            advance();
        }
        break;
    case '<':
        if (peek() == '<')
        {
            advance(); // Consume '<'
            if (peek() == '=')
            {
                advance();
            } // Consume '=' for <<=
        }
        else if (peek() == '=')
        {
            advance();
        } // Consume '=' for <=
        break;
    case '>':
        if (peek() == '>')
        {
            advance(); // Consume '>'
            if (peek() == '=')
            {
                advance();
            } // Consume '=' for >>=
        }
        else if (peek() == '=')
        {
            advance();
        } // Consume '=' for >=
        break;
    case '=':
        if (peek() == '=')
        {
            advance();
        } // Consume '=' for ==
        break;
    case '!':
        if (peek() == '=')
        {
            advance();
        } // Consume '=' for !=
        break;
    case '&':
        if (peek() == '&')
        {
            advance();
        }
        else if (peek() == '=')
        {
            advance();
        }
        break;
    case '|':
        if (peek() == '|')
        {
            advance();
        }
        else if (peek() == '=')
        {
            advance();
        }
        break;
    case '^':
        if (peek() == '=')
        {
            advance();
        }
        break;
    case '~':
        break; // Single char only
    case '?':
        break; // Single char only
    case ':':
        break; // Single char only
    default:
        // Should be unreachable if called correctly
        std::string val(1, c);
        return Token(TokenType::UNKNOWN, val, start_line, start_column);
    }
    // Determine token type based on the *consumed* string
    std::string op_value = source_.substr(start_pos, current_pos_ - start_pos);
    TokenType op_type = TokenType::UNKNOWN; // Default

    // Map the operator string to its type (more robust than nested ifs)
    static const std::unordered_map<std::string, TokenType> op_map = {
        {"+", TokenType::OP_PLUS}, {"-", TokenType::OP_MINUS}, {"*", TokenType::OP_STAR}, {"/", TokenType::OP_SLASH}, {"%", TokenType::OP_MOD}, {"<", TokenType::OP_LT}, {">", TokenType::OP_GT}, {"=", TokenType::OP_ASSIGN}, {"!", TokenType::OP_NOT}, {"&", TokenType::OP_BIT_AND}, {"|", TokenType::OP_BIT_OR}, {"^", TokenType::OP_BIT_XOR}, {"~", TokenType::OP_BIT_NOT}, {"?", TokenType::OP_QUESTION}, {":", TokenType::COLON}, {"++", TokenType::OP_INC}, {"--", TokenType::OP_DEC}, {"->", TokenType::ARROW}, {"+=", TokenType::OP_PLUS_ASSIGN}, {"-=", TokenType::OP_MINUS_ASSIGN}, {"*=", TokenType::OP_STAR_ASSIGN}, {"/=", TokenType::OP_SLASH_ASSIGN}, {"%=", TokenType::OP_MOD_ASSIGN}, {"<<", TokenType::OP_LSHIFT}, {">>", TokenType::OP_RSHIFT}, {"<=", TokenType::OP_LE}, {">=", TokenType::OP_GE}, {"==", TokenType::OP_EQ}, {"!=", TokenType::OP_NE}, {"&&", TokenType::OP_AND}, {"||", TokenType::OP_OR}, {"&=", TokenType::OP_AND_ASSIGN}, {"|=", TokenType::OP_OR_ASSIGN}, {"^=", TokenType::OP_XOR_ASSIGN}, {"<<=", TokenType::OP_LSHIFT_ASSIGN}, {">>=", TokenType::OP_RSHIFT_ASSIGN}};

    auto it = op_map.find(op_value);
    if (it != op_map.end())
    {
        op_type = it->second;
    }

    return Token(op_type, op_value, start_line, start_column);
}

// Scans a simple, single-character delimiter.
Token Lexer::scanDelimiter(int start_line, int start_column)
{
    char c = advance();    // Consume the delimiter
    std::string val(1, c); // Value is just the character itself

    switch (c)
    {
    case '(':
        return Token(TokenType::LPAREN, val, start_line, start_column);
    case ')':
        return Token(TokenType::RPAREN, val, start_line, start_column);
    case '{':
        return Token(TokenType::LBRACE, val, start_line, start_column);
    case '}':
        return Token(TokenType::RBRACE, val, start_line, start_column);
    case '[':
        return Token(TokenType::LBRACKET, val, start_line, start_column);
    case ']':
        return Token(TokenType::RBRACKET, val, start_line, start_column);
    case ';':
        return Token(TokenType::SEMICOLON, val, start_line, start_column);
    case ',':
        return Token(TokenType::COMMA, val, start_line, start_column);
    case '.':
        return Token(TokenType::DOT, val, start_line, start_column);

    default:
        // Should be unreachable
        return Token(TokenType::UNKNOWN, val, start_line, start_column);
    }
}

// Ensure lexAll stops if EOF is returned due to error limit
std::vector<Token> Lexer::lexAll()
{
    std::vector<Token> tokens;
    while (true)
    {
        Token token = getNextToken();
        tokens.push_back(token);
        // Stop adding tokens if EOF was generated due to error limit or actual EOF
        if (token.type == TokenType::EOF_TOKEN)
        {
            break;
        }
    }
    return tokens;
}