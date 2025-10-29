#include "lexer.h"

Lexer::Lexer(const std::string &source)
    : source_(source), current_pos_(0), current_line_(1), current_column_(1)
{
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

// Optimized: Combined whitespace and comment skipping in a single loop
void Lexer::skipWhitespaceAndComments()
{
    while (true)
    {
        char c = peek();

        // Skip whitespace characters
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            advance();
            continue;
        }

        // Check for comments
        if (c == '/')
        {
            size_t saved_pos = current_pos_;
            int saved_col = current_column_;
            advance(); // Consume '/'
            char next = peek();

            if (next == '/')
            {
                // Single-line comment: skip until newline or EOF
                while (peek() != '\n' && peek() != '\0')
                {
                    advance();
                }
                continue;
            }
            else if (next == '*')
            {
                // Multi-line comment: skip until */
                advance(); // Consume '*'
                while (true)
                {
                    if (peek() == '\0')
                    {
                        return; // Unterminated comment
                    }
                    if (peek() == '*')
                    {
                        advance();
                        if (peek() == '/')
                        {
                            advance(); // Consume '/'
                            break;    // Exit comment loop
                        }
                    }
                    else
                    {
                        advance();
                    }
                }
                continue;
            }
            else
            {
                // Not a comment, restore position
                current_pos_ = saved_pos;
                current_column_ = saved_col;
                return;
            }
        }

        // Not whitespace or comment
        return;
    }
}

Token Lexer::getNextToken()
{
    // --- 1. Skip whitespace and comments (optimized single pass) ---
    skipWhitespaceAndComments();

    // Store state *before* scanning the actual token
    int start_line = current_line_;
    int start_column = current_column_;
    string start_filename = current_filename_;

    char c = peek();

    // --- 2. Check for EOF ---
    if (c == '\0')
    {
        return Token(TokenType::EOF_TOKEN, "", start_filename, start_line, start_column);
    }

    // --- 3. Token Classification (optimized) ---

    // Identifiers and Keywords: [a-zA-Z_][a-zA-Z0-9_]*
    if (is_identifier_start(c))
    {
        result_token = scanIdentifierOrKeyword(start_line, start_column);
    }

    // Number Literals: 123, 0.5, 0x1A, .123
    // Check for digit OR '.' followed by a digit
    if (is_digit(c) || (c == '.' && current_pos_ + 1 < source_.length() && is_digit(source_[current_pos_ + 1])))
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
    // Note: '#' handled above
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
        advance(); // Consume the unknown character
        // Ensure the token value and type reflect the error
        result_token = Token(TokenType::UNKNOWN, std::string(1, c), start_filename, start_line, start_column);
    }

    // --- Increment error count if token is UNKNOWN ---
    if (result_token.type == TokenType::UNKNOWN)
    {
        error_count_++;
    }

    return result_token;
}

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

// Optimized: Uses substring extraction instead of character accumulation
Token Lexer::scanNumber(int start_line, int start_column)
{
    size_t start_pos = current_pos_;
    TokenType type = TokenType::INT_LITERAL; // Assume integer unless proven otherwise
    bool is_hex = false;
    size_t start_pos = current_pos_; // Remember start for final value string

    // --- 1. Handle Integer/Prefix Part ---
    if (peek() == '0')
    {
        advance(); // Consume '0'

        if (peek() == 'x' || peek() == 'X')
        {
            is_hex = true;
            advance(); // Consume 'x' or 'X'
            while (is_hex_digit(peek()))
            {
                advance();
            }
        }
        else
        {
            // Octal/Decimal '0': 0... (could be 0, 0123, 0.5)
            while (is_digit(peek()))
            {
                advance();
            }
        }
    }
    else if (is_digit(peek()))
    {
        // Decimal: [1-9]...
        while (is_digit(peek()))
        {
            advance();
        }
    }
    else if (peek() == '.')
    { // Float starting with '.'
        type = TokenType::FLOAT_LITERAL;
        advance(); // Consume '.'
        while (is_digit(peek()))
        {
            advance();
        }
    }
    else
    { // Should not happen
        return Token(TokenType::UNKNOWN, text, start_filename, start_line, start_column);
    }

    // --- 2. Handle Floating Point Parts (if not hex) ---
    if (!is_hex)
    {
        bool had_decimal = (type == TokenType::FLOAT_LITERAL); // Track if '.' started the number
        if (peek() == '.')
        {
            if (!had_decimal)
            { // Allow one '.'
                type = TokenType::FLOAT_LITERAL;
                advance(); // Consume '.'
                while (is_digit(peek()))
                {
                    advance();
                }
                had_decimal = true;
            } // Else: second '.', let end check handle
        }
        char exp_char = peek();
        if (exp_char == 'e' || exp_char == 'E')
        {
            type = TokenType::FLOAT_LITERAL;
            advance(); // Consume 'e' or 'E'

            if (peek() == '+' || peek() == '-')
            {
                advance();
            }
            while (is_digit(peek()))
            {
                advance();
            }
        }
    }

    // --- 3. Handle Suffixes ---
    std::string suffix_str; // Keep track of consumed suffixes
    if (type == TokenType::FLOAT_LITERAL)
    {
        char s = peek();
        // Only f/F/l/L allowed for non-hex floats
        if (!is_hex && (s == 'f' || s == 'F' || s == 'l' || s == 'L'))
        {
            advance();
        }
    }
    else
    {
        // Integer suffixes (U, L, LL) - order can be U, L, LL, UL, LU, etc.
        bool has_u = false;
        bool has_l = false;

        // Greedily consume suffixes
        if (peek() == 'u' || peek() == 'U')
        {
            advance();
            has_u = true;
        }

        if (peek() == 'l' || peek() == 'L')
        {
            advance();
            if (peek() == 'l' || peek() == 'L')
            {
                advance(); // Long Long
            }
            has_l = true;
        }

        // Check for 'U' again in case of "LU" order
        if (!has_u && (peek() == 'u' || peek() == 'U'))
        {
            advance();
        }

        // Check for 'L' again in case of "UL" order
        if (!has_l && (peek() == 'l' || peek() == 'L'))
        {
            advance();
            if (peek() == 'l' || peek() == 'L')
            {
                advance(); // Long Long
            }
        }
    }

    // Extract substring view (zero-copy)
    std::string_view value = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(type, value, start_line, start_column);
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

TokenType Lexer::checkKeyword(std::string_view value) const
{
    // Use transparent comparator for efficient string_view lookup
    std::string key(value);
    auto it = keywords_.find(key);
    if (it != keywords_.end())
    {
        return it->second; // It's a keyword
    }
    return TokenType::IDENTIFIER; // Not a keyword
}

// Optimized: Uses substring extraction instead of character accumulation
Token Lexer::scanIdentifierOrKeyword(int start_line, int start_column)
{
    size_t start_pos = current_pos_;
    advance(); // First character is already checked

    // Consume subsequent alphanumeric or underscore characters
    while (is_identifier_char(peek()))
    {
        advance();
    }

    // Extract substring view (zero-copy)
    std::string_view text = source_.substr(start_pos, current_pos_ - start_pos);

    // Check if the identifier is actually a keyword
    TokenType type = checkKeyword(text);
    return Token(type, text, start_line, start_column);
}

Token Lexer::scanCharLiteral(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    advance(); // Consume opening '

    if (peek() == '\'')
    {
        advance();
        return Token(TokenType::UNKNOWN, "''", start_filename, start_line, start_column);
    }
    if (peek() == '\n' || peek() == '\0')
    {
        return Token(TokenType::UNKNOWN, "'", start_filename, start_line, start_column);
    }

    if (peek() == '\\')
    {              // Escape sequence
        advance(); // Consume backslash
        if (peek() == '\0')
        {
            std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, raw_error, start_filename, start_line, start_column);
        }
        char escape_char = advance();
        switch (escape_char)
        {
        case 'n':
        case 't':
        case 'r':
        case '\\':
        case '\'':
        case '0':
            break; // Valid
        default:   // Invalid escape
            std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
            // Consume until closing quote or error? For now, just report invalid escape
            return Token(TokenType::UNKNOWN, raw_error, start_filename, start_line, start_column);
        }
    }
    else
    { // Regular character
        advance();
    }

    if (peek() != '\'')
    { // Multi-character or unterminated after char
        std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
        while (peek() != '\'' && peek() != '\n' && peek() != '\0')
        {
            raw_error += advance();
        }
        if (peek() == '\'')
        {
            raw_error += advance();
        }
        return Token(TokenType::UNKNOWN, raw_error, start_filename, start_line, start_column);
    }

    advance(); // Consume closing '

    // Valid char literal. Store the *actual* character value in processed_value
    return Token(TokenType::CHAR_LITERAL, "", std::string(1, actual_char), start_line, start_column);
}

Token Lexer::scanStringLiteral(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    advance(); // Consume opening "

    std::string processed_value; // The actual string value (escapes processed)
    processed_value.reserve(32);  // Pre-allocate to reduce reallocations

    while (peek() != '"')
    {
        if (peek() == '\n' || peek() == '\0')
        { // Unterminated string
            std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
            return Token(TokenType::UNKNOWN, raw_error, start_filename, start_line, start_column);
        }
        if (peek() == '\\')
        {              // Escape sequence
            advance(); // Consume '\'
            if (peek() == '\0')
            { // Unterminated escape
                std::string raw_error = source_.substr(start_pos, current_pos_ - start_pos);
                return Token(TokenType::UNKNOWN, raw_error, start_filename, start_line, start_column);
            }
            advance(); // Consume character after backslash (don't validate here)
        }
        else
        { // Regular character
            advance();
        }
    }
    advance(); // Consume closing "

    return Token(TokenType::STRING_LITERAL, "", std::move(processed_value), start_line, start_column);
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
    case '&':
    case '|':
    case '^':
        if (peek() == '=' || (c == '+' && peek() == '+') || (c == '-' && peek() == '-') || (c == '-' && peek() == '>'))
        {
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

    std::string op_value = source_.substr(start_pos, current_pos_ - start_pos);
    TokenType op_type = TokenType::UNKNOWN;

    // Map operator string to type
    static const std::unordered_map<std::string, TokenType> op_map = {
        {"+", TokenType::OP_PLUS}, {"-", TokenType::OP_MINUS}, {"*", TokenType::OP_STAR}, {"/", TokenType::OP_SLASH}, {"%", TokenType::OP_MOD}, {"<", TokenType::OP_LT}, {">", TokenType::OP_GT}, {"=", TokenType::OP_ASSIGN}, {"!", TokenType::OP_NOT}, {"&", TokenType::OP_BIT_AND}, {"|", TokenType::OP_BIT_OR}, {"^", TokenType::OP_BIT_XOR}, {"~", TokenType::OP_BIT_NOT}, {"?", TokenType::OP_QUESTION}, {":", TokenType::COLON}, {"++", TokenType::OP_INC}, {"--", TokenType::OP_DEC}, {"->", TokenType::ARROW}, {"+=", TokenType::OP_PLUS_ASSIGN}, {"-=", TokenType::OP_MINUS_ASSIGN}, {"*=", TokenType::OP_STAR_ASSIGN}, {"/=", TokenType::OP_SLASH_ASSIGN}, {"%=", TokenType::OP_MOD_ASSIGN}, {"<<", TokenType::OP_LSHIFT}, {">>", TokenType::OP_RSHIFT}, {"<=", TokenType::OP_LE}, {">=", TokenType::OP_GE}, {"==", TokenType::OP_EQ}, {"!=", TokenType::OP_NE}, {"&&", TokenType::OP_AND}, {"||", TokenType::OP_OR}, {"&=", TokenType::OP_AND_ASSIGN}, {"|=", TokenType::OP_OR_ASSIGN}, {"^=", TokenType::OP_XOR_ASSIGN}, {"<<=", TokenType::OP_LSHIFT_ASSIGN}, {">>=", TokenType::OP_RSHIFT_ASSIGN}};
    auto it = op_map.find(op_value);
    if (it != op_map.end())
    {
        op_type = it->second;
    }

    return Token(op_type, op_value, start_filename, start_line, start_column);
}

Token Lexer::scanDelimiter(int start_line, int start_column)
{
    string start_filename = current_filename_;
    char c = advance();
    std::string val(1, c);
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
        break; // Type remains UNKNOWN
    }
    return Token(type, val, start_filename, start_line, start_column);
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