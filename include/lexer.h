#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

enum class TokenType
{
    // we are going to start with 32 keywords
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

    // 19 Operators and Punctuators (Common C Operators)
    OP_ASSIGN, // =
    OP_EQ,     // ==
    OP_NE,     // !=
    OP_LT,     // <
    OP_LE,     // <=
    OP_GT,     // >
    OP_GE,     // >=
    OP_PLUS,   // +
    OP_MINUS,  // -
    OP_STAR,   // *
    OP_SLASH,  // /
    OP_MOD,    // %
    OP_INC,    // ++
    OP_DEC,    // --
    OP_LSHIFT, // <<
    OP_RSHIFT, // >>
    OP_AND,    // &&
    OP_OR,     // ||
    OP_NOT,    // !

    // Bitwise Operators
    OP_BIT_AND,  // &
    OP_BIT_OR,   // |
    OP_BIT_XOR,  // ^
    OP_BIT_NOT,  // ~

    // Compound Assignment Operators
    OP_PLUS_ASSIGN,   // +=
    OP_MINUS_ASSIGN,  // -=
    OP_STAR_ASSIGN,   // *=
    OP_SLASH_ASSIGN,  // /=
    OP_MOD_ASSIGN,    // %=
    OP_AND_ASSIGN,    // &=
    OP_OR_ASSIGN,     // |=
    OP_XOR_ASSIGN,    // ^=
    OP_LSHIFT_ASSIGN, // <<=
    OP_RSHIFT_ASSIGN, // >>=

    // Ternary/Conditional
    OP_QUESTION, // ?

    // 11 Delimiters
    LPAREN,    // (
    RPAREN,    // )
    LBRACE,    // {
    RBRACE,    // }
    LBRACKET,  // [
    RBRACKET,  // ]
    SEMICOLON, // ;
    COMMA,     // ,
    COLON,     // :
    DOT,       // .
    ARROW,     // ->

    // Special Tokens
    EOF_TOKEN, // End of File
    UNKNOWN    // For lexical errors
};

string token_type_to_string(TokenType);

struct Token
{
    TokenType type;
    string value; // Stores the raw text
    int line;          // 1-based line number
    int column;        // 1-based column number where the token starts

    Token(TokenType type, const string &value, int line, int column);

    string to_string() const;
};

/**
 * @brief Reads C source code and converts it into a stream of Tokens.
 */
class Lexer
{
public:
    /*
       Constructs a Lexer for a given C source string.
       The complete string containing the source code.
     */
    Lexer(const string &source);

    /*
       Returns the next recognized token, advancing the internal position.
       The next Token in the stream.
     */
    Token getNextToken();

    /*
       Collects all tokens from the source until EOF is reached.
       A vector of all recognized Tokens.
     */
    vector<Token> lexAll();

private:
    // Lexer State
    const string source_; // The source code being tokenized
    size_t current_pos_;       // Index into source_ for the current character

    // Location tracking (updated by advance() and skipWhitespace())
    int current_line_;
    int current_column_;

    // Core Lexing Primitives
    char peek() const;
    char advance();
    void skipWhitespace();
    void skipComment();

    // Token Scanning Functions
    Token scanIdentifierOrKeyword(int start_line, int start_column);
    Token scanNumber(int start_line, int start_column);
    Token scanStringLiteral(int start_line, int start_column);

    // Helper for distinguishing keywords from identifiers
    TokenType checkKeyword(const string &value) const;

    //lookup table to map keywords
    static const unordered_map<string, TokenType> keywords_;
};

#endif