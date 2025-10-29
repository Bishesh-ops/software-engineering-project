#include "lexer.h"
#include <sstream>

// Constructor updated to include filename parameter 'fname'
Token::Token(TokenType type, const string &value, const string &fname, int line, int column)
    : type(type), value(value), filename(fname), line(line), column(column) {} // <-- FIXED: Added fname parameter

// Creates a human-readable string representation (for debugging).
std::string Token::to_string() const
{
    std::ostringstream oss;
    oss << "Token(" << token_type_to_string(type) << ", \"";

    // Escape control characters for clean printing
    for (char c : value)
    {
        switch (c)
        {
        case '\n':
            oss << "\\n";
            break;
        case '\t':
            oss << "\\t";
            break;
        case '\r':
            oss << "\\r";
            break;
        case '\0':
            oss << "\\0";
            break;
        case '\\':
            oss << "\\\\";
            break;
        case '"':
            oss << "\\\"";
            break;
        default:
            oss << c;
            break;
        }
    }

    // Include filename in output
    oss << "\", file: \"" << filename << "\", line: " << line << ", column: " << column << ")";
    return oss.str();
}

// Converts a TokenType enum to its string representation (for debugging).
std::string token_type_to_string(TokenType type)
{
    switch (type)
    {
    // Keywords
    case TokenType::KW_AUTO:
        return "KW_AUTO";
    case TokenType::KW_BREAK:
        return "KW_BREAK";
    case TokenType::KW_CASE:
        return "KW_CASE";
    case TokenType::KW_CHAR:
        return "KW_CHAR";
    case TokenType::KW_CONST:
        return "KW_CONST";
    case TokenType::KW_CONTINUE:
        return "KW_CONTINUE";
    case TokenType::KW_DEFAULT:
        return "KW_DEFAULT";
    case TokenType::KW_DO:
        return "KW_DO";
    case TokenType::KW_DOUBLE:
        return "KW_DOUBLE";
    case TokenType::KW_ELSE:
        return "KW_ELSE";
    case TokenType::KW_ENUM:
        return "KW_ENUM";
    case TokenType::KW_EXTERN:
        return "KW_EXTERN";
    case TokenType::KW_FLOAT:
        return "KW_FLOAT";
    case TokenType::KW_FOR:
        return "KW_FOR";
    case TokenType::KW_GOTO:
        return "KW_GOTO";
    case TokenType::KW_IF:
        return "KW_IF";
    case TokenType::KW_INT:
        return "KW_INT";
    case TokenType::KW_LONG:
        return "KW_LONG";
    case TokenType::KW_REGISTER:
        return "KW_REGISTER";
    case TokenType::KW_RETURN:
        return "KW_RETURN";
    case TokenType::KW_SHORT:
        return "KW_SHORT";
    case TokenType::KW_SIGNED:
        return "KW_SIGNED";
    case TokenType::KW_SIZEOF:
        return "KW_SIZEOF";
    case TokenType::KW_STATIC:
        return "KW_STATIC";
    case TokenType::KW_STRUCT:
        return "KW_STRUCT";
    case TokenType::KW_SWITCH:
        return "KW_SWITCH";
    case TokenType::KW_TYPEDEF:
        return "KW_TYPEDEF";
    case TokenType::KW_UNION:
        return "KW_UNION";
    case TokenType::KW_UNSIGNED:
        return "KW_UNSIGNED";
    case TokenType::KW_VOID:
        return "KW_VOID";
    case TokenType::KW_VOLATILE:
        return "KW_VOLATILE";
    case TokenType::KW_WHILE:
        return "KW_WHILE";

    // Identifiers and Literals
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::INT_LITERAL:
        return "INT_LITERAL";
    case TokenType::FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case TokenType::STRING_LITERAL:
        return "STRING_LITERAL";
    case TokenType::CHAR_LITERAL:
        return "CHAR_LITERAL";

    // Operators
    case TokenType::OP_ASSIGN:
        return "OP_ASSIGN"; // =
    case TokenType::OP_EQ:
        return "OP_EQ"; // ==
    case TokenType::OP_NE:
        return "OP_NE"; // !=
    case TokenType::OP_LT:
        return "OP_LT"; // <
    case TokenType::OP_LE:
        return "OP_LE"; // <=
    case TokenType::OP_GT:
        return "OP_GT"; // >
    case TokenType::OP_GE:
        return "OP_GE"; // >=
    case TokenType::OP_PLUS:
        return "OP_PLUS"; // +
    case TokenType::OP_MINUS:
        return "OP_MINUS"; // -
    case TokenType::OP_STAR:
        return "OP_STAR"; // *
    case TokenType::OP_SLASH:
        return "OP_SLASH"; // /
    case TokenType::OP_MOD:
        return "OP_MOD"; // %
    case TokenType::OP_INC:
        return "OP_INC"; // ++
    case TokenType::OP_DEC:
        return "OP_DEC"; // --
    case TokenType::OP_LSHIFT:
        return "OP_LSHIFT"; // <<
    case TokenType::OP_RSHIFT:
        return "OP_RSHIFT"; // >>
    case TokenType::OP_AND:
        return "OP_AND"; // &&
    case TokenType::OP_OR:
        return "OP_OR"; // ||
    case TokenType::OP_NOT:
        return "OP_NOT"; // !

    // Bitwise Operators
    case TokenType::OP_BIT_AND:
        return "OP_BIT_AND"; // &
    case TokenType::OP_BIT_OR:
        return "OP_BIT_OR"; // |
    case TokenType::OP_BIT_XOR:
        return "OP_BIT_XOR"; // ^
    case TokenType::OP_BIT_NOT:
        return "OP_BIT_NOT"; // ~

    // Compound Assignment Operators
    case TokenType::OP_PLUS_ASSIGN:
        return "OP_PLUS_ASSIGN"; // +=
    case TokenType::OP_MINUS_ASSIGN:
        return "OP_MINUS_ASSIGN"; // -=
    case TokenType::OP_STAR_ASSIGN:
        return "OP_STAR_ASSIGN"; // *=
    case TokenType::OP_SLASH_ASSIGN:
        return "OP_SLASH_ASSIGN"; // /=
    case TokenType::OP_MOD_ASSIGN:
        return "OP_MOD_ASSIGN"; // %=
    case TokenType::OP_AND_ASSIGN:
        return "OP_AND_ASSIGN"; // &=
    case TokenType::OP_OR_ASSIGN:
        return "OP_OR_ASSIGN"; // |=
    case TokenType::OP_XOR_ASSIGN:
        return "OP_XOR_ASSIGN"; // ^=
    case TokenType::OP_LSHIFT_ASSIGN:
        return "OP_LSHIFT_ASSIGN"; // <<=
    case TokenType::OP_RSHIFT_ASSIGN:
        return "OP_RSHIFT_ASSIGN"; // >>=

    // Ternary/Conditional
    case TokenType::OP_QUESTION:
        return "OP_QUESTION"; // ?

    // Delimiters
    case TokenType::LPAREN:
        return "LPAREN"; // (
    case TokenType::RPAREN:
        return "RPAREN"; // )
    case TokenType::LBRACE:
        return "LBRACE"; // {
    case TokenType::RBRACE:
        return "RBRACE"; // }
    case TokenType::LBRACKET:
        return "LBRACKET"; // [
    case TokenType::RBRACKET:
        return "RBRACKET"; // ]
    case TokenType::SEMICOLON:
        return "SEMICOLON"; // ;
    case TokenType::COMMA:
        return "COMMA"; // ,
    case TokenType::COLON:
        return "COLON"; // :
    case TokenType::DOT:
        return "DOT"; // .
    case TokenType::ARROW:
        return "ARROW"; // ->

    // Preprocessor Tokens
    case TokenType::HASH:
        return "HASH";
    case TokenType::DOUBLE_HASH:
        return "DOUBLE_HASH";

    // Special Tokens
    case TokenType::EOF_TOKEN:
        return "EOF_TOKEN";
    case TokenType::UNKNOWN:
        return "UNKNOWN";

    default:
        return "UNKNOWN"; // Should be unreachable
    }
}