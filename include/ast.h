#ifndef AST_H
#define AST_H
#include <string>
#include <memory>
#include <vector>

// Forward Declarations
class ASTVisitor;
class Expression;
class Statement;
class Declaration;
class BinaryExpr;
class UnaryExpr;
class LiteralExpr;
class IdentifierExpr;
class CallExpr;
class AssignmentExpr;
class ArrayAccessExpr;
class IfStmt;
class WhileStmt;
class ForStmt;
class ReturnStmt;
class CompoundStmt;
class ExpressionStmt;
class VarDecl;
class TypeDecl;
class StructDecl;
class FunctionDecl;
class ParameterDecl;

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
    ASSIGNMENT_EXPR,
    ARRAY_ACCESS_EXPR,

    // Statement types
    IF_STMT,
    WHILE_STMT,
    FOR_STMT,
    RETURN_STMT,
    COMPOUND_STMT,
    EXPRESSION_STMT,

    // Declaration types
    VAR_DECL,
    TYPE_DECL,
    STRUCT_DECL,

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

// ============================================================================
// Base ASTNode Class
// ============================================================================
class ASTNode
{
protected:
    ASTNodeType nodeType;
    SourceLocation location;

public:
    ASTNode(ASTNodeType type, const SourceLocation &loc) : nodeType(type), location(loc) {}
    virtual ~ASTNode() = default;

    // Visitor pattern support
    virtual void accept(ASTVisitor &visitor) = 0;

    // Getters
    ASTNodeType getNodeType() const { return nodeType; }
    const SourceLocation &getLocation() const { return location; }
    int getLine() const { return location.line; }
    int getColumn() const { return location.column; }
};

// ============================================================================
// Visitor Pattern Interface
// ============================================================================
class ASTVisitor
{
public:
    virtual ~ASTVisitor() = default;

    // Expression visitors
    virtual void visit(BinaryExpr &node) = 0;
    virtual void visit(UnaryExpr &node) = 0;
    virtual void visit(LiteralExpr &node) = 0;
    virtual void visit(IdentifierExpr &node) = 0;
    virtual void visit(CallExpr &node) = 0;
    virtual void visit(AssignmentExpr &node) = 0;
    virtual void visit(ArrayAccessExpr &node) = 0;

    // Statement visitors
    virtual void visit(IfStmt &node) = 0;
    virtual void visit(WhileStmt &node) = 0;
    virtual void visit(ForStmt &node) = 0;
    virtual void visit(ReturnStmt &node) = 0;
    virtual void visit(CompoundStmt &node) = 0;
    virtual void visit(ExpressionStmt &node) = 0;

    // Declaration visitors
    virtual void visit(VarDecl &node) = 0;
    virtual void visit(TypeDecl &node) = 0;
    virtual void visit(StructDecl &node) = 0;
    virtual void visit(FunctionDecl &node) = 0;
    virtual void visit(ParameterDecl &node) = 0;
};

// ============================================================================
// Base Category Classes
// ============================================================================

// Base Expression class
class Expression : public ASTNode
{
public:
    Expression(ASTNodeType type, const SourceLocation &loc) : ASTNode(type, loc) {}
    virtual ~Expression() = default;
};

// Base Statement class
class Statement : public ASTNode
{
public:
    Statement(ASTNodeType type, const SourceLocation &loc) : ASTNode(type, loc) {}
    virtual ~Statement() = default;
};

// Base Declaration class
class Declaration : public ASTNode
{
public:
    Declaration(ASTNodeType type, const SourceLocation &loc) : ASTNode(type, loc) {}
    virtual ~Declaration() = default;
};

// ============================================================================
// Expression Node Classes
// ============================================================================

// Binary Expression (e.g., a + b, x * y)
class BinaryExpr : public Expression
{
private:
    std::unique_ptr<Expression> left;
    std::string op; // operator: +, -, *, /, ==, !=, <, >, etc.
    std::unique_ptr<Expression> right;

public:
    BinaryExpr(std::unique_ptr<Expression> lhs, std::string operation,
               std::unique_ptr<Expression> rhs, const SourceLocation &loc)
        : Expression(ASTNodeType::BINARY_EXPR, loc),
          left(std::move(lhs)), op(operation), right(std::move(rhs)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getLeft() const { return left.get(); }
    Expression *getRight() const { return right.get(); }
    const std::string &getOperator() const { return op; }
};

// Unary Expression (e.g., -x, !flag, *ptr, &var)
class UnaryExpr : public Expression
{
private:
    std::string op; // operator: -, !, *, &, ++, --, etc.
    std::unique_ptr<Expression> operand;
    bool prefixOp;  // true for prefix (++x), false for postfix (x++)

public:
    UnaryExpr(std::string operation, std::unique_ptr<Expression> expr,
              bool prefix, const SourceLocation &loc)
        : Expression(ASTNodeType::UNARY_EXPR, loc),
          op(operation), operand(std::move(expr)), prefixOp(prefix) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getOperand() const { return operand.get(); }
    const std::string &getOperator() const { return op; }
    bool isPrefixOp() const { return prefixOp; }
};

// Literal Expression (e.g., 42, 3.14, "hello", 'c')
class LiteralExpr : public Expression
{
public:
    enum class LiteralType
    {
        INTEGER,
        FLOAT,
        STRING,
        CHAR,
        BOOLEAN
    };

private:
    std::string value;
    LiteralType literalType;

public:
    LiteralExpr(std::string val, LiteralType type, const SourceLocation &loc)
        : Expression(ASTNodeType::LITERAL_EXPR, loc), value(val), literalType(type) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getValue() const { return value; }
    LiteralType getLiteralType() const { return literalType; }
};

// Identifier Expression (e.g., variable name, function name)
class IdentifierExpr : public Expression
{
private:
    std::string name;

public:
    IdentifierExpr(std::string identifier, const SourceLocation &loc)
        : Expression(ASTNodeType::IDENTIFIER_EXPR, loc), name(identifier) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getName() const { return name; }
};

// Function Call Expression (e.g., foo(a, b, c))
class CallExpr : public Expression
{
private:
    std::unique_ptr<Expression> callee; // function being called
    std::vector<std::unique_ptr<Expression>> arguments;

public:
    CallExpr(std::unique_ptr<Expression> func,
             std::vector<std::unique_ptr<Expression>> args,
             const SourceLocation &loc)
        : Expression(ASTNodeType::CALL_EXPR, loc),
          callee(std::move(func)), arguments(std::move(args)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getCallee() const { return callee.get(); }
    const std::vector<std::unique_ptr<Expression>> &getArguments() const { return arguments; }
};

// Assignment Expression (e.g., x = 5)
class AssignmentExpr : public Expression
{
private:
    std::unique_ptr<Expression> target;  // left-hand side (usually identifier)
    std::unique_ptr<Expression> value;   // right-hand side

public:
    AssignmentExpr(std::unique_ptr<Expression> lhs,
                   std::unique_ptr<Expression> rhs,
                   const SourceLocation &loc)
        : Expression(ASTNodeType::ASSIGNMENT_EXPR, loc),
          target(std::move(lhs)), value(std::move(rhs)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getTarget() const { return target.get(); }
    Expression *getValue() const { return value.get(); }
};

// Array Access Expression (e.g., arr[index] or arr[i + 1])
class ArrayAccessExpr : public Expression
{
private:
    std::unique_ptr<Expression> array;  // array being accessed
    std::unique_ptr<Expression> index;  // index expression

public:
    ArrayAccessExpr(std::unique_ptr<Expression> arr,
                    std::unique_ptr<Expression> idx,
                    const SourceLocation &loc)
        : Expression(ASTNodeType::ARRAY_ACCESS_EXPR, loc),
          array(std::move(arr)), index(std::move(idx)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getArray() const { return array.get(); }
    Expression *getIndex() const { return index.get(); }
};

// ============================================================================
// Statement Node Classes
// ============================================================================

// If Statement (if-else)
class IfStmt : public Statement
{
private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch; // can be nullptr

public:
    IfStmt(std::unique_ptr<Expression> cond,
           std::unique_ptr<Statement> thenStmt,
           std::unique_ptr<Statement> elseStmt,
           const SourceLocation &loc)
        : Statement(ASTNodeType::IF_STMT, loc),
          condition(std::move(cond)), thenBranch(std::move(thenStmt)),
          elseBranch(std::move(elseStmt)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getCondition() const { return condition.get(); }
    Statement *getThenBranch() const { return thenBranch.get(); }
    Statement *getElseBranch() const { return elseBranch.get(); }
};

// While Statement
class WhileStmt : public Statement
{
private:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

public:
    WhileStmt(std::unique_ptr<Expression> cond,
              std::unique_ptr<Statement> bodyStmt,
              const SourceLocation &loc)
        : Statement(ASTNodeType::WHILE_STMT, loc),
          condition(std::move(cond)), body(std::move(bodyStmt)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getCondition() const { return condition.get(); }
    Statement *getBody() const { return body.get(); }
};

// For Statement
class ForStmt : public Statement
{
private:
    std::unique_ptr<Statement> initializer; // can be VarDecl or Expression
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<Statement> body;

public:
    ForStmt(std::unique_ptr<Statement> init,
            std::unique_ptr<Expression> cond,
            std::unique_ptr<Expression> inc,
            std::unique_ptr<Statement> bodyStmt,
            const SourceLocation &loc)
        : Statement(ASTNodeType::FOR_STMT, loc),
          initializer(std::move(init)), condition(std::move(cond)),
          increment(std::move(inc)), body(std::move(bodyStmt)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Statement *getInitializer() const { return initializer.get(); }
    Expression *getCondition() const { return condition.get(); }
    Expression *getIncrement() const { return increment.get(); }
    Statement *getBody() const { return body.get(); }
};

// Return Statement
class ReturnStmt : public Statement
{
private:
    std::unique_ptr<Expression> returnValue; // can be nullptr for void return

public:
    ReturnStmt(std::unique_ptr<Expression> value, const SourceLocation &loc)
        : Statement(ASTNodeType::RETURN_STMT, loc), returnValue(std::move(value)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getReturnValue() const { return returnValue.get(); }
};

// Compound Statement (block of statements)
class CompoundStmt : public Statement
{
private:
    std::vector<std::unique_ptr<Statement>> statements;

public:
    CompoundStmt(std::vector<std::unique_ptr<Statement>> stmts, const SourceLocation &loc)
        : Statement(ASTNodeType::COMPOUND_STMT, loc), statements(std::move(stmts)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::vector<std::unique_ptr<Statement>> &getStatements() const { return statements; }
};

// Expression Statement (expression used as a statement, e.g., x = 5;)
class ExpressionStmt : public Statement
{
private:
    std::unique_ptr<Expression> expression;

public:
    ExpressionStmt(std::unique_ptr<Expression> expr, const SourceLocation &loc)
        : Statement(ASTNodeType::EXPRESSION_STMT, loc), expression(std::move(expr)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    Expression *getExpression() const { return expression.get(); }
};

// ============================================================================
// Declaration Node Classes
// ============================================================================

// Variable Declaration (e.g., int x = 5; or int arr[10]; or int *ptr;)
class VarDecl : public Declaration
{
private:
    std::string name;
    std::string type;
    std::unique_ptr<Expression> initializer; // can be nullptr
    bool isArray;                             // true if this is an array declaration
    std::unique_ptr<Expression> arraySize;    // size expression for arrays (can be nullptr)
    int pointerLevel;                         // number of pointer indirections (0 = not a pointer, 1 = *, 2 = **, etc.)

public:
    VarDecl(std::string varName, std::string varType,
            std::unique_ptr<Expression> init, const SourceLocation &loc,
            bool array = false, std::unique_ptr<Expression> size = nullptr,
            int ptrLevel = 0)
        : Declaration(ASTNodeType::VAR_DECL, loc),
          name(varName), type(varType), initializer(std::move(init)),
          isArray(array), arraySize(std::move(size)), pointerLevel(ptrLevel) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getName() const { return name; }
    const std::string &getType() const { return type; }
    Expression *getInitializer() const { return initializer.get(); }
    bool getIsArray() const { return isArray; }
    Expression *getArraySize() const { return arraySize.get(); }
    int getPointerLevel() const { return pointerLevel; }
    bool isPointer() const { return pointerLevel > 0; }
};

// Type Declaration (e.g., typedef)
class TypeDecl : public Declaration
{
private:
    std::string name;
    std::string underlyingType;

public:
    TypeDecl(std::string typeName, std::string baseType, const SourceLocation &loc)
        : Declaration(ASTNodeType::TYPE_DECL, loc), name(typeName), underlyingType(baseType) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getName() const { return name; }
    const std::string &getUnderlyingType() const { return underlyingType; }
};

// Struct Declaration
class StructDecl : public Declaration
{
private:
    std::string name;
    std::vector<std::unique_ptr<VarDecl>> fields;

public:
    StructDecl(std::string structName, std::vector<std::unique_ptr<VarDecl>> members,
               const SourceLocation &loc)
        : Declaration(ASTNodeType::STRUCT_DECL, loc), name(structName), fields(std::move(members)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getName() const { return name; }
    const std::vector<std::unique_ptr<VarDecl>> &getFields() const { return fields; }
};

// Function Declaration
class FunctionDecl : public Declaration
{
private:
    std::string name;
    std::string returnType;
    std::vector<std::unique_ptr<ParameterDecl>> parameters;
    std::unique_ptr<CompoundStmt> body; // can be nullptr for forward declarations

public:
    FunctionDecl(std::string funcName, std::string retType,
                 std::vector<std::unique_ptr<ParameterDecl>> params,
                 std::unique_ptr<CompoundStmt> funcBody,
                 const SourceLocation &loc)
        : Declaration(ASTNodeType::FUNCTION_DECL, loc),
          name(funcName), returnType(retType),
          parameters(std::move(params)), body(std::move(funcBody)) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getName() const { return name; }
    const std::string &getReturnType() const { return returnType; }
    const std::vector<std::unique_ptr<ParameterDecl>> &getParameters() const { return parameters; }
    CompoundStmt *getBody() const { return body.get(); }
};

// Parameter Declaration (for function parameters)
class ParameterDecl : public Declaration
{
private:
    std::string name;
    std::string type;

public:
    ParameterDecl(std::string paramName, std::string paramType, const SourceLocation &loc)
        : Declaration(ASTNodeType::PARAMETER_DECL, loc), name(paramName), type(paramType) {}

    void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

    const std::string &getName() const { return name; }
    const std::string &getType() const { return type; }
};

#endif