#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <string_view> // Use string_view
#include <iostream>
#include <vector>
#include <unordered_map>
#include "error_handler.h"

using namespace std; // Avoid in headers if possible

// Defines all possible token types the lexer can produce.
enum class TokenType
{
    // C89 Keywords (32)
    KW_AUTO,
    KW_BREAK,
    KW_CASE,
    KW_CHAR,
    KW_CONST,
    KW_CONTINUE,
    KW_DEFAULT,
    KW_DO,
    KW_DOUBLE,
    KW_ELSE,
    KW_ENUM,
    KW_EXTERN,
    KW_FLOAT,
    KW_FOR,
    KW_GOTO,
    KW_IF,
    KW_INT,
    KW_LONG,
    KW_REGISTER,
    KW_RETURN,
    KW_SHORT,
    KW_SIGNED,
    KW_SIZEOF,
    KW_STATIC,
    KW_STRUCT,
    KW_SWITCH,
    KW_TYPEDEF,
    KW_UNION,
    KW_UNSIGNED,
    KW_VOID,
    KW_VOLATILE,
    KW_WHILE,

    // Identifiers and Literals
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,

    // Operators
    OP_ASSIGN,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_PLUS,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,
    OP_MOD,
    OP_INC,
    OP_DEC,
    OP_LSHIFT,
    OP_RSHIFT,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_BIT_NOT,

    // Compound Assignment Operators
    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_STAR_ASSIGN,
    OP_SLASH_ASSIGN,
    OP_MOD_ASSIGN,
    OP_AND_ASSIGN,
    OP_OR_ASSIGN,
    OP_XOR_ASSIGN,
    OP_LSHIFT_ASSIGN,
    OP_RSHIFT_ASSIGN,

    // Ternary/Conditional
    OP_QUESTION, // ?

    // Delimiters and Separators
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    COMMA,
    COLON,
    DOT,
    ARROW,

    // Preprocessor Tokens
    HASH,        // #
    DOUBLE_HASH, // ##

    // Special Tokens
    EOF_TOKEN, // End of File
    UNKNOWN    // Lexical error
};

std::string token_type_to_string(TokenType type);

// Represents a single lexical unit (token) found in the source code.
struct Token
{
    TokenType type;
    std::string_view value;      // Zero-copy view into source (lexeme)
    std::string processed_value; // Storage for processed strings (char/string literals)
    std::string filename;        // The source filename for this token
    int line;                    // 1-based line number in that file
    int column;                  // 1-based column number where the token starts

    // Constructor for tokens primarily using string_view
    Token(TokenType type, std::string_view value, const std::string &fname, int line, int column);

    // Constructor for tokens needing separate processed storage (like string/char literals)
    Token(TokenType type, std::string_view value, std::string processed, const std::string &fname, int line, int column);

    std::string to_string() const;
};

// Performs lexical analysis (tokenization) on a C source string.
class Lexer
{
public:
    // Constructor accepts initial source and filename
    Lexer(const std::string &source, const std::string &initial_filename = "input");
    Token getNextToken();
    std::vector<Token> lexAll();

    // Reset lexer position to beginning (for re-use after lexAll())
    void reset();

    // Access to error handler
    ErrorHandler& getErrorHandler() { return error_handler_; }
    const ErrorHandler& getErrorHandler() const { return error_handler_; }
    bool hasErrors() const { return error_handler_.has_errors(); }

private:
    // --- Lexer State ---
    const std::string source_; // Store the source string (not a view)
    const std::string_view source_view_; // View into the stored source
    size_t current_pos_;
    int current_line_;
    int current_column_;
    std::string current_filename_; // Track current filename
    ErrorHandler error_handler_; // Unified error reporting
    static const int MAX_ERRORS = 100; // Increased for better error recovery

    // --- Core Lexing Primitives (Optimized) ---
    // Peek with bounds check
    inline char peek() const
    {
        return (current_pos_ < source_view_.length()) ? source_view_[current_pos_] : '\0';
    }
    // Peek without bounds check (use only when sure it's safe)
    inline char peek_unchecked() const
    {
        return source_view_[current_pos_];
    }
    // Peek ahead N characters with bounds check
    inline char peek(size_t n) const
    {
        return (current_pos_ + n < source_view_.length()) ? source_view_[current_pos_ + n] : '\0';
    }

    char advance();
    void skipWhitespace();
    void skipComment();
    void skipRestOfLine();
    bool handleLineDirective();

    // --- Character Classification (Optimized) ---
    static inline bool is_identifier_start(char c)
    {
        return std::isalpha(c) || c == '_';
    }
    static inline bool is_identifier_char(char c)
    {
        return std::isalnum(c) || c == '_';
    }
    static inline bool is_digit(char c)
    {
        return std::isdigit(c);
    }
    static inline bool is_hex_digit(char c)
    {
        return std::isxdigit(c);
    }

    // --- Token Scanning Functions ---
    Token scanIdentifierOrKeyword(int start_line, int start_column);
    Token scanNumber(int start_line, int start_column);
    Token scanCharLiteral(int start_line, int start_column);
    Token scanStringLiteral(int start_line, int start_column);
    Token scanOperator(int start_line, int start_column);
    Token scanDelimiter(int start_line, int start_column);

    // Update checkKeyword to accept string_view
    TokenType checkKeyword(std::string_view value) const;

    // C keyword lookup table
    static const std::unordered_map<std::string, TokenType> keywords_;
};

#endif // LEXER_H