#include "lexer.h"
#include <cctype>    // for isalpha, isalnum, isdigit, isxdigit
#include <string>    // For std::stoi
#include <stdexcept> // For exception handling in stoi

// Initialize filename and error_count_ in the constructor
Lexer::Lexer(const std::string &source, const std::string &initial_filename)
    : source_(source), current_pos_(0), current_line_(1), current_column_(1),
      current_filename_(initial_filename), error_count_(0)
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

// Helper to consume characters until the next newline or EOF
void Lexer::skipRestOfLine()
{
    while (peek() != '\n' && peek() != '\0')
    {
        advance();
    }
}

// Attempts to parse a #line directive after the # has been consumed.
// Returns true if a #line directive was successfully processed, false otherwise.
bool Lexer::handleLineDirective()
{
    size_t directive_start_pos = current_pos_; // Remember position after #
    int directive_start_col = current_column_;
    string directive_start_fname = current_filename_; // Capture state at start
    int directive_start_line = current_line_;

    skipWhitespace();

    // Check for the "line" identifier
    if (!(std::isalpha(peek()) || peek() == '_'))
    {
        // Not an identifier, cannot be #line
        current_pos_ = directive_start_pos; // Backtrack
        current_column_ = directive_start_col;
        return false; // Let getNextToken handle # or ##
    }

    // Temporarily scan the identifier without creating a token yet
    size_t kw_start_pos = current_pos_;
    advance();
    while (std::isalnum(peek()) || peek() == '_')
    {
        advance();
    }
    std::string identifier = source_.substr(kw_start_pos, current_pos_ - kw_start_pos);

    if (identifier != "line")
    {
        // It was some other directive like #define, backtrack fully
        current_pos_ = directive_start_pos; // Backtrack to just after #
        current_column_ = directive_start_col;
        return false; // Let getNextToken handle # or ##
    }

    // --- It is a #line directive, now parse the arguments ---

    skipWhitespace();

    // Expect line number (integer literal)
    if (!std::isdigit(peek()))
    {
        // Malformed #line directive (missing line number)
        error_count_++;
        skipRestOfLine(); // Skip the rest of the bad directive
        return true;      // Indicate directive was handled (even though errored)
    }
    // Scan the number token just to get its value string
    Token line_num_token = scanNumber(current_line_, current_column_);
    if (line_num_token.type != TokenType::INT_LITERAL)
    {
        // Malformed #line directive (not an integer)
        error_count_++;
        // scanNumber might have consumed more than just the number if invalid
        // Need to ensure we skip the rest of the directive line
        // A simple approach: back up to where scanNumber started and skip line
        current_pos_ = kw_start_pos + identifier.length(); // Position after 'line' kw
        // Re-skip whitespace after 'line' before skipping line
        skipWhitespace();
        skipRestOfLine();
        return true;
    }

    // Expect optional filename (string literal)
    skipWhitespace();
    std::string new_filename = current_filename_; // Keep current if none provided
    if (peek() == '"')
    {
        Token filename_token = scanStringLiteral(current_line_, current_column_);
        if (filename_token.type == TokenType::STRING_LITERAL)
        {
            // Unescape the raw string value from the token
            if (filename_token.value.length() >= 2)
            {
                new_filename = filename_token.value.substr(1, filename_token.value.length() - 2);
                // TODO: A more robust implementation would properly unescape sequences inside the filename string
            }
            else
            {
                error_count_++; // Malformed filename string ""
            }
        }
        else
        {
            error_count_++; // Malformed #line (expected filename string, got UNKNOWN)
                            // scanStringLiteral already advanced past error, continue skipping line
        }
    } // If not '"', filename is omitted, new_filename remains current_filename_

    // Now update the lexer's state
    try
    {
        int new_line = std::stoi(line_num_token.value);
        // The #line directive applies to the *next* line.
        // Adjust the lexer's current line number so that the *next* newline encountered
        // correctly sets the line number reported in subsequent tokens.
        current_line_ = new_line - 1; // Set to line *before* the target line
        // Column is typically reset conceptually, though not strictly needed here
        // as the newline handling in advance() will reset it.
        current_filename_ = new_filename;
    }
    catch (const std::exception &e)
    {
        // Error converting line number (e.g., too large)
        error_count_++;
        // State remains unchanged, skip rest of line
    }

    // Consume the rest of the #line directive line
    skipRestOfLine();
    // The *next* call to getNextToken will handle the newline character itself.

    return true; // Successfully processed (or attempted to process) #line
}

Token Lexer::getNextToken()
{
    if (error_count_ >= MAX_ERRORS)
    {
        // Pass current filename even for early EOF
        return Token(TokenType::EOF_TOKEN, "", current_filename_, current_line_, current_column_);
    }

    while (true)
    {
        skipWhitespace();
        while (peek() == '\n')
        {
            advance(); // advance handles line/col update
            skipWhitespace();
        }

        // --- Handle directives '#' ---
        if (peek() == '#')
        {
            int hash_line = current_line_; // Store line where # starts
            int hash_col = current_column_;
            string hash_filename = current_filename_; // Capture filename too
            advance();                                // Consume '#'

            if (handleLineDirective())
            {
                // #line directive was processed and skipped, state updated.
                // Loop again to find the next *actual* token after the directive's newline.
                continue;
            }
            else if (peek() == '#')
            {
                // It's a double hash ##
                advance();                                                                      // Consume second '#'
                return Token(TokenType::DOUBLE_HASH, "##", hash_filename, hash_line, hash_col); // Use captured state
            }
            else
            {
                // It's just a single hash # (or start of another directive like #define)
                return Token(TokenType::HASH, "#", hash_filename, hash_line, hash_col); // Use captured state
            }
        }
        // --- End Directive Handling ---

        // --- Handle Comments '/' ---
        if (peek() == '/')
        {
            advance(); // Consume the first '/'
            if (peek() == '/' || peek() == '*')
            {
                skipComment();
                continue; // Restart loop after comment
            }
            else
            {
                // It's just an operator, put back '/'
                current_pos_--;
                current_column_--;
                break; // Proceed to operator scanning below
            }
        }
        else
        {
            // Not whitespace, newline, #, or comment - proceed to token
            break;
        }
    } // End of skipping loop

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

    // --- 3. Token Classification ---
    Token result_token = Token(TokenType::UNKNOWN, "", start_filename, start_line, start_column); // Default

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

// --- All Scan Functions MUST use the correct Token Constructor ---

Token Lexer::scanIdentifierOrKeyword(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    advance(); // First char
    while (std::isalnum(peek()) || peek() == '_')
    {
        advance();
    }
    std::string final_value = source_.substr(start_pos, current_pos_ - start_pos);
    TokenType type = checkKeyword(final_value);
    return Token(type, final_value, start_filename, start_line, start_column); // Pass filename
}

Token Lexer::scanNumber(int start_line, int start_column)
{
    string start_filename = current_filename_;
    std::string text; // Stores the consumed characters for error reporting if needed
    TokenType type = TokenType::INT_LITERAL;
    bool is_hex = false;
    size_t start_pos = current_pos_; // Remember start for final value string

    // --- 1. Handle Integer/Prefix Part ---
    if (peek() == '0')
    {
        text += advance();
        if (peek() == 'x' || peek() == 'X')
        {
            is_hex = true;
            text += advance();
            if (!std::isxdigit(peek()))
            { // Error: 0x must have hex digits
                return Token(TokenType::UNKNOWN, text, start_filename, start_line, start_column);
            }
            while (std::isxdigit(peek()))
            {
                text += advance();
            }
        }
        else
        { // Octal or float 0.x
            while (std::isdigit(peek()))
            {
                if (peek() >= '8')
                {
                    break;
                } // Invalid octal digit, treat as decimal 0 potentially followed by error
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
        text += advance();
        if (!std::isdigit(peek()))
        { // Error: '.' must have digits
            return Token(TokenType::UNKNOWN, text, start_filename, start_line, start_column);
        }
        while (std::isdigit(peek()))
        {
            text += advance();
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
                text += advance();
                while (std::isdigit(peek()))
                {
                    text += advance();
                }
                had_decimal = true;
            } // Else: second '.', let end check handle
        }
        char exp_char = peek();
        if (exp_char == 'e' || exp_char == 'E')
        {
            char prev_char = text.empty() ? '\0' : text.back();
            if (std::isdigit(prev_char) || prev_char == '.')
            { // Exponent must follow digit or '.'
                type = TokenType::FLOAT_LITERAL;
                text += advance();
                if (peek() == '+' || peek() == '-')
                {
                    text += advance();
                }
                if (!std::isdigit(peek()))
                { // Error: Exponent needs digits
                    while (std::isalnum(peek()))
                    {
                        text += advance();
                    } // Consume bad part
                    std::string full_text = source_.substr(start_pos, current_pos_ - start_pos);
                    return Token(TokenType::UNKNOWN, full_text, start_filename, start_line, start_column);
                }
                while (std::isdigit(peek()))
                {
                    text += advance();
                }
            } // Else: 'e' not after digit/'.', let end check handle
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
            suffix_str += advance();
        }
    }
    else
    { // Integer suffixes
        bool has_u = false, has_l = false, has_ll = false;
        for (int i = 0; i < 3; ++i)
        { // Max 3 suffix chars (ULL)
            char s = peek();
            if (!has_u && (s == 'u' || s == 'U'))
            {
                has_u = true;
                suffix_str += advance();
            }
            else if (!has_ll && (s == 'l' || s == 'L'))
            {
                if (has_l)
                    has_ll = true; // Mark as LL if L seen before
                has_l = true;
                suffix_str += advance();
            }
            else
            {
                break; // Not a valid integer suffix char
            }
        }
    }
    text += suffix_str; // Append suffix to potentially erroneous text

    // --- FINAL CHECK: Check for invalid characters immediately following ---
    if (std::isalnum(peek()) || peek() == '.')
    {
        // Consume the invalid part
        while (std::isalnum(peek()) || peek() == '.')
        {
            text += advance();
        }
        std::string full_text = source_.substr(start_pos, current_pos_ - start_pos);
        return Token(TokenType::UNKNOWN, full_text, start_filename, start_line, start_column);
    }

    // Valid number - construct the final value string correctly
    std::string final_value = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(type, final_value, start_filename, start_line, start_column);
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
    std::string raw_value = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(TokenType::CHAR_LITERAL, raw_value, start_filename, start_line, start_column);
}

Token Lexer::scanStringLiteral(int start_line, int start_column)
{
    string start_filename = current_filename_;
    size_t start_pos = current_pos_;
    advance(); // Consume opening "

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
    std::string raw_value = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(TokenType::STRING_LITERAL, raw_value, start_filename, start_line, start_column);
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