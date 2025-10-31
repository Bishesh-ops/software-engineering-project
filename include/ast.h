#ifndef AST_H
#define AST_H
#include <string>
#include <memory>
#include <vector>

// Forward Declarations

class ASTVisitor;

enum class ASTNodeType
{
    // Base categories
    EXPRESSION,
    STATEMENT,
    DECLARATION,
    FUNCTION,

    // Expression types
    BINARY_EXPR,
    UNARY_EXPR,
    LITERAL_EXPR,
    IDENTIFIER_EXPR,
    CALL_EXPR,

    // Statement types
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    COMPOUND_STMT,

    // Declaration types
    VAR_DECL,
    TYPE_DECL,
    STRUCT_DEC,

    // Function types
    FUNCTION_DECL,
    PARAMETER_DECL,

};

// Source location info for error reporting
struct SourceLocation
{
    std::string filename;
    int line;
    int column;
    SourceLocation(const std::string &fname = "", int ln = 0, int col = 0) : filename(fname), line(ln), column(col) {}
    std::string toString() const
    {
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

#endif