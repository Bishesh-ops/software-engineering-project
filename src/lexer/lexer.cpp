#include "lexer.h"
#include <string>    // For std::string, std::stoi
#include <stdexcept> // For exception handling
#include <cctype>    // For std::isdigit, std::isalpha, etc.

// Constructor: Stores the source string and creates a view into it
Lexer::Lexer(const std::string &source, const std::string &initial_filename)
    : source_(source), // Store a copy of the source string
      source_view_(source_), // Create a view into the stored string
      current_pos_(0),
      current_line_(1),
      current_column_(1),
      current_filename_(initial_filename),
      error_handler_()
{
    error_handler_.set_max_errors(MAX_ERRORS);
    // Register source code for error context display
    error_handler_.register_source(initial_filename, source_);
}

// --- Core Lexing Primitives ---

char Lexer::advance()
{
    if (current_pos_ >= source_view_.length())
    {
        return '\0';
    }

    char current_char = source_view_[current_pos_]; // Use string_view access
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
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r')
        { // Skip space, tab, cr
            advance();
        }
        else
        {
            break;
        }
    }
}

// Skips C-style comments (// and /* */)
// Assumes the initial '/' has already been consumed by the caller.
void Lexer::skipComment()
{
    if (peek() == '/')
    {              // Single-line comment
        advance(); // Consume the second /
        skipRestOfLine();
    }
    else if (peek() == '*')
    {              // Multi-line comment
        advance(); // Consume '*'
        while (true)
        {
            if (peek() == '\0')
                break; // Unterminated
            // --- CORRECTED: Check for '*' FIRST ---
            if (peek() == '*')
            {
                advance(); // Consume '*'
                if (peek() == '/')
                {              // Check if the NEXT char is '/'
                    advance(); // Consume closing '/'
                    break;     // End of comment
                }
                // If not '/', it was just a '*', continue loop
                // (No need to put it back, already advanced)
            }
            else
            {
                advance(); // Consume char inside comment (handles newlines correctly)
            }
            // --- End Correction ---
        }
    }
    // If neither '/' nor '*', the caller handles the single '/' operator
}

void Lexer::skipRestOfLine()
{
    while (peek() != '\n' && peek() != '\0')
    {
        advance();
    }
}

// --- #line Directive Handling ---
bool Lexer::handleLineDirective()
{
    size_t directive_start_pos = current_pos_; // Remember position after #
    int directive_start_col = current_column_;

    skipWhitespace(); // Use the standard whitespace skip

    if (!is_identifier_start(peek()))
    { // Must start with identifier
        current_pos_ = directive_start_pos;
        current_column_ = directive_start_col;
        return false;
    }

    // Scan the potential "line" keyword
    size_t kw_start_pos = current_pos_;
    advance();
    while (is_identifier_char(peek()))
    {
        advance();
    }
    string_view identifier = source_view_.substr(kw_start_pos, current_pos_ - kw_start_pos);

    if (identifier != "line")
    {
        // Backtrack to just after the #
        current_pos_ = directive_start_pos;
        current_column_ = directive_start_col;
        return false; // Let getNextToken handle # or ##
    }

    // --- It is a #line directive, parse arguments ---
    skipWhitespace();

    // Expect line number
    if (!is_digit(peek()))
    {
        error_handler_.error("Malformed #line directive: expected line number",
                            SourceLocation(current_filename_, current_line_, current_column_));
        skipRestOfLine();
        return true; // Malformed
    }
    Token line_num_token = scanNumber(current_line_, current_column_);
    std::string line_num_str(line_num_token.value); // Convert view to string for stoi
    if (line_num_token.type != TokenType::INT_LITERAL)
    {
        error_handler_.error("Malformed #line directive: invalid line number",
                            SourceLocation(current_filename_, current_line_, current_column_));
        skipRestOfLine();
        return true; // Malformed
    }

    // Expect optional filename
    skipWhitespace();
    std::string new_filename = current_filename_;
    if (peek() == '"')
    {
        Token filename_token = scanStringLiteral(current_line_, current_column_);
        if (filename_token.type == TokenType::STRING_LITERAL)
        {
            // Use processed_value which has escapes handled
            if (!filename_token.processed_value.empty())
            {
                new_filename = filename_token.processed_value;
            }
            else if (filename_token.value.length() >= 2)
            { // Fallback: strip quotes
                new_filename = string(filename_token.value.substr(1, filename_token.value.length() - 2));
            }
            else
            {
                error_handler_.error("Malformed #line directive: empty filename",
                                    SourceLocation(current_filename_, current_line_, current_column_));
            } // Malformed ""
        }
        else
        {
            error_handler_.error("Malformed #line directive: expected filename string",
                                SourceLocation(current_filename_, current_line_, current_column_));
        } // Malformed, expected string
    }

    // Update lexer state
    try
    {
        int new_line = std::stoi(line_num_str);
        // Set line number to one *before* the directive's number,
        // because the newline *after* the directive will increment it.
        current_line_ = new_line - 1;
        current_filename_ = new_filename;
    }
    catch (const std::exception &e)
    {
        error_handler_.error("Malformed #line directive: " + string(e.what()),
                            SourceLocation(current_filename_, current_line_, current_column_));
    }

    skipRestOfLine();
    return true; // Indicate directive was handled
}

// --- Main Tokenization Function ---
Token Lexer::getNextToken()
{
    // Remove hard stop on MAX_ERRORS - just track errors but continue tokenizing
    // This allows better error recovery and complete tokenization

    // --- 1. Skip whitespace, newlines, and comments ---
    while (true)
    {
        skipWhitespace(); // Skip spaces, tabs, \r

        if (peek() == '\n')
        {
            advance(); // Consume newline, updates line/col
            continue;  // Go back to skip potential whitespace on new line
        }

        if (peek() == '/')
        {
            char next = peek(1);
            if (next == '/' || next == '*')
            {
                advance(); // Consume the first '/'
                skipComment();
                continue; // Restart skipping loop
            }
            // else: It's a division operator, break loop
        }
        // If not whitespace, newline, or comment, break loop
        break;
    }

    // Store state *before* scanning the actual token
    int start_line = current_line_;
    int start_column = current_column_;
    string start_filename = current_filename_;
    size_t start_pos = current_pos_; // Needed for string_view

    char c = peek();

    // --- 2. Check for EOF ---
    if (c == '\0')
    {
        return Token(TokenType::EOF_TOKEN, source_view_.substr(current_pos_), start_filename, start_line, start_column);
    }

    // --- 3. Handle '#' Directives Specially ---
    if (c == '#')
    {
        advance(); // Consume '#'
        if (handleLineDirective())
        {
            // #line was handled, state updated, skipped rest of line.
            // Get the *next* real token recursively.
            return getNextToken();
        }
        else if (peek() == '#')
        {
            advance(); // Consume second '#'
            string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::DOUBLE_HASH, val, start_filename, start_line, start_column);
        }
        else
        {
            // Single hash (e.g., #define)
            string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::HASH, val, start_filename, start_line, start_column);
        }
    }

    // --- 4. Token Classification (using inline helpers) ---
    Token result_token = Token(TokenType::UNKNOWN, "", start_filename, start_line, start_column); // Default

    // Identifiers and Keywords
    if (is_identifier_start(c))
    {
        result_token = scanIdentifierOrKeyword(start_line, start_column);
    }
    // Number Literals (Check for digit OR '.' followed by a digit)
    else if (is_digit(c) || (c == '.' && is_digit(peek(1))))
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
    // Operators
    else if (string_view("+-*/%<>&|^=!~?:").find(c) != string_view::npos)
    {
        result_token = scanOperator(start_line, start_column);
    }
    // Delimiters
    else if (string_view("(){}[].,;").find(c) != string_view::npos)
    {
        result_token = scanDelimiter(start_line, start_column);
    }
    // --- 5. Handle Unrecognized Characters ---
    else
    {
        advance(); // Consume the unknown character
        string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
        result_token = Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
    }

    // --- Report error if token is UNKNOWN ---
    if (result_token.type == TokenType::UNKNOWN)
    {
        string error_msg = "Unknown or invalid token: '";
        error_msg += string(result_token.value);
        error_msg += "'";
        error_handler_.error(error_msg,
                            SourceLocation(result_token.filename, result_token.line, result_token.column));
    }

    return result_token;
}

// --- lexAll ---
vector<Token> Lexer::lexAll()
{
    vector<Token> tokens;
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

// --- Scanning Functions (Updated for string_view) ---

Token Lexer::scanIdentifierOrKeyword(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    advance(); // First char already checked
    while (is_identifier_char(peek()))
    {
        advance();
    }

    string_view value = source_view_.substr(start_pos, current_pos_ - start_pos);
    TokenType type = checkKeyword(value);
    return Token(type, value, start_filename, start_line, start_column);
}

Token Lexer::scanNumber(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    TokenType type = TokenType::INT_LITERAL;
    bool is_hex = false;

    // --- 1. Integer/Prefix Part ---
    if (peek() == '0')
    {
        advance();
        if (peek() == 'x' || peek() == 'X')
        {
            is_hex = true;
            advance();
            if (!is_hex_digit(peek()))
            { // Error: 0x must have digits
                string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
                return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
            }
            while (is_hex_digit(peek()))
            {
                advance();
            }
        }
        else
        { // Octal or float 0.x
            while (is_digit(peek()))
            {
                if (peek() >= '8')
                {
                    break;
                } // Invalid octal digit
                advance();
            }
        }
    }
    else if (is_digit(peek()))
    { // Decimal
        while (is_digit(peek()))
        {
            advance();
        }
    }
    else if (peek() == '.')
    { // Float starting with '.'
        type = TokenType::FLOAT_LITERAL;
        advance();
        if (!is_digit(peek()))
        { // Error: '.' must have digits
            string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
        }
        while (is_digit(peek()))
        {
            advance();
        }
    }

    // --- 2. Floating Point Parts ---
    if (!is_hex)
    {
        bool had_decimal = (type == TokenType::FLOAT_LITERAL);
        if (peek() == '.')
        {
            if (!had_decimal)
            {
                type = TokenType::FLOAT_LITERAL;
                advance();
                while (is_digit(peek()))
                {
                    advance();
                }
                had_decimal = true;
            } // Else: second '.', let end check handle
        }
        if (peek() == 'e' || peek() == 'E')
        {
            char prev_char = (current_pos_ > start_pos) ? source_view_[current_pos_ - 1] : '\0';
            if (is_digit(prev_char) || prev_char == '.')
            {
                type = TokenType::FLOAT_LITERAL;
                advance();
                if (peek() == '+' || peek() == '-')
                {
                    advance();
                }
                if (!is_digit(peek()))
                { // Error: Exponent needs digits
                    while (std::isalnum(peek_unchecked()))
                    {
                        advance();
                    }
                    string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
                    return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
                }
                while (is_digit(peek()))
                {
                    advance();
                }
            } // Else: 'e' not after digit/'.', let end check handle
        }
    }

    // --- 3. Suffixes ---
    if (type == TokenType::FLOAT_LITERAL)
    {
        char s = peek();
        if (!is_hex && (s == 'f' || s == 'F' || s == 'l' || s == 'L'))
        {
            advance();
        }
    }
    else
    { // Integer suffixes
        bool has_u = false, has_l = false, has_ll = false;
        for (int i = 0; i < 3; ++i)
        {
            char s = peek();
            if (!has_u && (s == 'u' || s == 'U'))
            {
                has_u = true;
                advance();
            }
            else if (!has_ll && (s == 'l' || s == 'L'))
            {
                if (has_l)
                    has_ll = true;
                has_l = true;
                advance();
            }
            else
            {
                break;
            }
        }
    }

    // --- FINAL CHECK ---
    if (std::isalnum(peek()) || peek() == '.')
    {
        while (std::isalnum(peek()) || peek() == '.')
        {
            advance();
        }
        string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
        return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
    }

    // Valid number
    string_view value = source_view_.substr(start_pos, current_pos_ - start_pos);
    return Token(type, value, start_filename, start_line, start_column);
}

Token Lexer::scanCharLiteral(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    string processed_value; // Store processed char here
    advance();              // Consume opening '

    if (peek() == '\'')
    {
        advance();
        error_handler_.error("Empty character literal",
                            SourceLocation(start_filename, start_line, start_column));
        return Token(TokenType::UNKNOWN, source_view_.substr(start_pos, current_pos_ - start_pos), start_filename, start_line, start_column);
    }
    if (peek() == '\n' || peek() == '\0')
    {
        error_handler_.error("Unterminated character literal",
                            SourceLocation(start_filename, start_line, start_column));
        return Token(TokenType::UNKNOWN, source_view_.substr(start_pos, current_pos_ - start_pos), start_filename, start_line, start_column);
    }

    char actual_char;
    if (peek() == '\\')
    {
        advance(); // Consume backslash
        if (peek() == '\0')
        {
            error_handler_.error("Unterminated escape sequence in character literal",
                                SourceLocation(start_filename, start_line, start_column));
            string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
        }
        char escape_char = advance();
        switch (escape_char)
        { // Process escape
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
        default: // Invalid escape
            error_handler_.error("Invalid escape sequence '\\" + string(1, escape_char) + "' in character literal",
                                SourceLocation(start_filename, start_line, start_column));
            string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
        }
        processed_value += actual_char; // Store processed char
    }
    else
    {
        actual_char = advance(); // Consume & store regular char
        processed_value += actual_char;
    }

    if (peek() != '\'')
    { // Error: multi-char or unterminated
        error_handler_.error("Multi-character or unterminated character literal",
                            SourceLocation(start_filename, start_line, start_column));
        while (peek() != '\'' && peek() != '\n' && peek() != '\0')
        {
            advance();
        }
        if (peek() == '\'')
        {
            advance();
        }
        string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
        return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
    }

    advance(); // Consume closing '
    string_view value = source_view_.substr(start_pos, current_pos_ - start_pos);
    // Use the constructor that stores the processed char
    return Token(TokenType::CHAR_LITERAL, value, processed_value, start_filename, start_line, start_column);
}

Token Lexer::scanStringLiteral(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    advance();                   // Consume opening "
    string processed_value;      // Store processed string here
    processed_value.reserve(32); // Pre-allocate

    while (peek() != '"')
    {
        if (peek() == '\n' || peek() == '\0')
        { // Unterminated string
            error_handler_.error("Unterminated string literal",
                                SourceLocation(start_filename, start_line, start_column));
            string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
        }
        if (peek() == '\\')
        {              // Escape sequence
            advance(); // Consume '\'
            if (peek() == '\0')
            { // Unterminated escape
                error_handler_.error("Unterminated escape sequence in string literal",
                                    SourceLocation(start_filename, start_line, start_column));
                string_view val = source_view_.substr(start_pos, current_pos_ - start_pos);
                return Token(TokenType::UNKNOWN, val, start_filename, start_line, start_column);
            }
            char escape_char = advance(); // Consume char after backslash
            // Process common escapes
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
            default:
                processed_value += escape_char;
                break; // Treat invalid as literal
            }
        }
        else
        { // Regular character
            processed_value += advance();
        }
    }
    advance(); // Consume closing "
    string_view value = source_view_.substr(start_pos, current_pos_ - start_pos);
    // Use constructor with processed value
    return Token(TokenType::STRING_LITERAL, value, std::move(processed_value), start_filename, start_line, start_column);
}

Token Lexer::scanOperator(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    char c = advance(); // Consume first char

    // Consume potential second/third chars
    switch (c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=':
    case '!':
    case '|':
    case '^': // Added '|' and '^' here
        // Check for '=', '++', '--', '->'
        if (peek() == '=' || (c == '+' && peek() == '+') || (c == '-' && peek() == '-') || (c == '-' && peek() == '>'))
        {
            advance();
        }
        break;
    case '&': // Special case for & and &&
        if (peek() == '&' || peek() == '=')
        { // Correctly check for && or &=
            advance();
        }
        break;
    case '<':
    case '>':
        if (peek() == c)
        { // << or >>
            advance();
            if (peek() == '=')
            {
                advance();
            } // <<= or >>=
        }
        else if (peek() == '=')
        { // <= or >=
            advance();
        }
        break;
    // Single char operators: ~ ? : fall through
    case '~':
    case '?':
    case ':':
    default:
        break;
    }

    string_view value = source_view_.substr(start_pos, current_pos_ - start_pos);
    TokenType op_type = TokenType::UNKNOWN;

    // Map operator string to type (using string conversion for lookup)
    static const std::unordered_map<std::string, TokenType> op_map = {
        {"+", TokenType::OP_PLUS}, {"-", TokenType::OP_MINUS}, {"*", TokenType::OP_STAR}, {"/", TokenType::OP_SLASH}, {"%", TokenType::OP_MOD}, {"<", TokenType::OP_LT}, {">", TokenType::OP_GT}, {"=", TokenType::OP_ASSIGN}, {"!", TokenType::OP_NOT}, {"&", TokenType::OP_BIT_AND}, {"|", TokenType::OP_BIT_OR}, {"^", TokenType::OP_BIT_XOR}, {"~", TokenType::OP_BIT_NOT}, {"?", TokenType::OP_QUESTION}, {":", TokenType::COLON}, {"++", TokenType::OP_INC}, {"--", TokenType::OP_DEC}, {"->", TokenType::ARROW}, {"+=", TokenType::OP_PLUS_ASSIGN}, {"-=", TokenType::OP_MINUS_ASSIGN}, {"*=", TokenType::OP_STAR_ASSIGN}, {"/=", TokenType::OP_SLASH_ASSIGN}, {"%=", TokenType::OP_MOD_ASSIGN}, {"<<", TokenType::OP_LSHIFT}, {">>", TokenType::OP_RSHIFT}, {"<=", TokenType::OP_LE}, {">=", TokenType::OP_GE}, {"==", TokenType::OP_EQ}, {"!=", TokenType::OP_NE}, {"&&", TokenType::OP_AND}, // Correctly mapped
        {"||", TokenType::OP_OR},
        {"&=", TokenType::OP_AND_ASSIGN},
        {"|=", TokenType::OP_OR_ASSIGN},
        {"^=", TokenType::OP_XOR_ASSIGN},
        {"<<=", TokenType::OP_LSHIFT_ASSIGN},
        {">>=", TokenType::OP_RSHIFT_ASSIGN}};
    std::string op_key(value); // Convert string_view for map lookup
    auto it = op_map.find(op_key);
    if (it != op_map.end())
    {
        op_type = it->second;
    }
    // If not found, op_type remains UNKNOWN

    return Token(op_type, value, start_filename, start_line, start_column);
}

Token Lexer::scanDelimiter(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    char c = advance();
    string_view value = source_view_.substr(start_pos, current_pos_ - start_pos);
    TokenType type = TokenType::UNKNOWN;

    switch (c)
    {
    case '(':
        type = TokenType::LPAREN;
        break;
    case ')':
        type = TokenType::RPAREN;
        break;
    case '{':
        type = TokenType::LBRACE;
        break;
    case '}':
        type = TokenType::RBRACE;
        break;
    case '[':
        type = TokenType::LBRACKET;
        break;
    case ']':
        type = TokenType::RBRACKET;
        break;
    case ';':
        type = TokenType::SEMICOLON;
        break;
    case ',':
        type = TokenType::COMMA;
        break;
    case '.':
        type = TokenType::DOT;
        break;
    default:
        break;
    }
    return Token(type, value, start_filename, start_line, start_column);
}

// Keyword lookup (needs string_view key or conversion)
TokenType Lexer::checkKeyword(string_view value) const
{
    // Convert string_view to string for lookup
    std::string key(value);
    auto it = keywords_.find(key);
    if (it != keywords_.end())
    {
        return it->second; // It's a keyword
    }
    return TokenType::IDENTIFIER; // Not a keyword
}

// Static map definition (remains the same)
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"auto", TokenType::KW_AUTO}, {"break", TokenType::KW_BREAK}, {"case", TokenType::KW_CASE}, {"char", TokenType::KW_CHAR}, {"const", TokenType::KW_CONST}, {"continue", TokenType::KW_CONTINUE}, {"default", TokenType::KW_DEFAULT}, {"do", TokenType::KW_DO}, {"double", TokenType::KW_DOUBLE}, {"else", TokenType::KW_ELSE}, {"enum", TokenType::KW_ENUM}, {"extern", TokenType::KW_EXTERN}, {"float", TokenType::KW_FLOAT}, {"for", TokenType::KW_FOR}, {"goto", TokenType::KW_GOTO}, {"if", TokenType::KW_IF}, {"int", TokenType::KW_INT}, {"long", TokenType::KW_LONG}, {"register", TokenType::KW_REGISTER}, {"return", TokenType::KW_RETURN}, {"short", TokenType::KW_SHORT}, {"signed", TokenType::KW_SIGNED}, {"sizeof", TokenType::KW_SIZEOF}, {"static", TokenType::KW_STATIC}, {"struct", TokenType::KW_STRUCT}, {"switch", TokenType::KW_SWITCH}, {"typedef", TokenType::KW_TYPEDEF}, {"union", TokenType::KW_UNION}, {"unsigned", TokenType::KW_UNSIGNED}, {"void", TokenType::KW_VOID}, {"volatile", TokenType::KW_VOLATILE}, {"while", TokenType::KW_WHILE}};