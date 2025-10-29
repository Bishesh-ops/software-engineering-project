#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <string_view>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

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

    // ... (rest of enum remains the same) ...
    EOF_TOKEN, // End of File
    UNKNOWN    // Lexical error
};

string token_type_to_string(TokenType);

// Represents a single lexical unit (token) found in the source code.
struct Token
{
    TokenType type;
    string_view value;     // Zero-copy view into source (lexeme) for most tokens
    string processed_value; // Storage for processed strings (char/string literals with escapes)
    int line;              // 1-based line number
    int column;            // 1-based column number where the token starts

    // Constructor for tokens using string_view (zero-copy)
    Token(TokenType type, string_view value, int line, int column);

    // Constructor for tokens needing processed storage (char/string literals)
    Token(TokenType type, string_view value, string processed, int line, int column);

    string to_string() const;
};

// Performs lexical analysis (tokenization) on a C source string.
class Lexer
{
public:
    Lexer(const string &source, const string &initial_filename = "input"); // <-- ADDED initial_filename
    Token getNextToken();
    vector<Token> lexAll();

private:
    // --- Lexer State ---
    const string_view source_;  // Zero-copy view of source
    size_t current_pos_;
    int current_line_;
    int current_column_;
    string current_filename_; // <-- ADDED: Track current filename
    int error_count_;
    static const int MAX_ERRORS = 10;

    // --- Core Lexing Primitives (Optimized) ---
    inline char peek() const {
        return (current_pos_ < source_.length()) ? source_[current_pos_] : '\0';
    }

    inline char peek_unchecked() const {
        return source_[current_pos_];
    }

    char advance();
    void skipWhitespaceAndComments();

    // --- Character Classification (Optimized) ---
    static inline bool is_identifier_start(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    static inline bool is_identifier_char(char c) {
        return is_identifier_start(c) || (c >= '0' && c <= '9');
    }

    static inline bool is_digit(char c) {
        return c >= '0' && c <= '9';
    }

    static inline bool is_hex_digit(char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    // --- Token Scanning Functions ---
    // (These now need to create Tokens with the filename)
    Token scanIdentifierOrKeyword(int start_line, int start_column);
    Token scanNumber(int start_line, int start_column);
    Token scanCharLiteral(int start_line, int start_column);
    Token scanStringLiteral(int start_line, int start_column);
    Token scanOperator(int start_line, int start_column);
    Token scanDelimiter(int start_line, int start_column);

    TokenType checkKeyword(string_view value) const;

    // C keyword lookup table (using transparent comparator for string_view)
    static const unordered_map<string, TokenType> keywords_;
};

#endif // LEXER_H