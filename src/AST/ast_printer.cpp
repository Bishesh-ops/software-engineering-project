#include "../../include/ast.h"
#include <iostream>

// ============================================================================
// AST Printer - Example Visitor Implementation
// ============================================================================

/*
 * ASTPrinter: A concrete visitor that prints the AST structure
 * -------------------------------------------------------------
 * This class demonstrates how to implement the visitor pattern for AST traversal.
 * It prints a human-readable, indented representation of the AST for debugging.
 *
 * Usage:
 *   ASTPrinter printer;
 *   astRoot->accept(printer);
 *
 * The printer uses indentation to show the tree structure visually.
 */

class ASTPrinter : public ASTVisitor
{
private:
    int indentLevel = 0;
    const int INDENT_SIZE = 2;

    void printIndent()
    {
        for (int i = 0; i < indentLevel * INDENT_SIZE; i++)
        {
            std::cout << " ";
        }
    }

    void increaseIndent() { indentLevel++; }
    void decreaseIndent() { indentLevel--; }

public:
    // ========================================================================
    // Expression Visitors
    // ========================================================================

    void visit(BinaryExpr &node) override
    {
        printIndent();
        std::cout << "BinaryExpr(" << node.getOperator() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        printIndent();
        std::cout << "Left:" << std::endl;
        increaseIndent();
        node.getLeft()->accept(*this);
        decreaseIndent();

        printIndent();
        std::cout << "Right:" << std::endl;
        increaseIndent();
        node.getRight()->accept(*this);
        decreaseIndent();
        decreaseIndent();
    }

    void visit(UnaryExpr &node) override
    {
        printIndent();
        std::cout << "UnaryExpr(" << node.getOperator() << ", "
                  << (node.isPrefixOp() ? "prefix" : "postfix") << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        node.getOperand()->accept(*this);
        decreaseIndent();
    }

    void visit(LiteralExpr &node) override
    {
        printIndent();
        std::cout << "LiteralExpr(";

        switch (node.getLiteralType())
        {
        case LiteralExpr::LiteralType::INTEGER:
            std::cout << "INT: ";
            break;
        case LiteralExpr::LiteralType::FLOAT:
            std::cout << "FLOAT: ";
            break;
        case LiteralExpr::LiteralType::STRING:
            std::cout << "STRING: ";
            break;
        case LiteralExpr::LiteralType::CHAR:
            std::cout << "CHAR: ";
            break;
        case LiteralExpr::LiteralType::BOOLEAN:
            std::cout << "BOOL: ";
            break;
        }

        std::cout << node.getValue() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;
    }

    void visit(IdentifierExpr &node) override
    {
        printIndent();
        std::cout << "IdentifierExpr(" << node.getName() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;
    }

    void visit(CallExpr &node) override
    {
        printIndent();
        std::cout << "CallExpr "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        printIndent();
        std::cout << "Callee:" << std::endl;
        increaseIndent();
        node.getCallee()->accept(*this);
        decreaseIndent();

        printIndent();
        std::cout << "Arguments (" << node.getArguments().size() << "):" << std::endl;
        increaseIndent();
        for (const auto &arg : node.getArguments())
        {
            arg->accept(*this);
        }
        decreaseIndent();
        decreaseIndent();
    }

    // ========================================================================
    // Statement Visitors
    // ========================================================================

    void visit(IfStmt &node) override
    {
        printIndent();
        std::cout << "IfStmt "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        printIndent();
        std::cout << "Condition:" << std::endl;
        increaseIndent();
        node.getCondition()->accept(*this);
        decreaseIndent();

        printIndent();
        std::cout << "Then:" << std::endl;
        increaseIndent();
        node.getThenBranch()->accept(*this);
        decreaseIndent();

        if (node.getElseBranch())
        {
            printIndent();
            std::cout << "Else:" << std::endl;
            increaseIndent();
            node.getElseBranch()->accept(*this);
            decreaseIndent();
        }
        decreaseIndent();
    }

    void visit(WhileStmt &node) override
    {
        printIndent();
        std::cout << "WhileStmt "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        printIndent();
        std::cout << "Condition:" << std::endl;
        increaseIndent();
        node.getCondition()->accept(*this);
        decreaseIndent();

        printIndent();
        std::cout << "Body:" << std::endl;
        increaseIndent();
        node.getBody()->accept(*this);
        decreaseIndent();
        decreaseIndent();
    }

    void visit(ForStmt &node) override
    {
        printIndent();
        std::cout << "ForStmt "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        if (node.getInitializer())
        {
            printIndent();
            std::cout << "Initializer:" << std::endl;
            increaseIndent();
            node.getInitializer()->accept(*this);
            decreaseIndent();
        }

        if (node.getCondition())
        {
            printIndent();
            std::cout << "Condition:" << std::endl;
            increaseIndent();
            node.getCondition()->accept(*this);
            decreaseIndent();
        }

        if (node.getIncrement())
        {
            printIndent();
            std::cout << "Increment:" << std::endl;
            increaseIndent();
            node.getIncrement()->accept(*this);
            decreaseIndent();
        }

        printIndent();
        std::cout << "Body:" << std::endl;
        increaseIndent();
        node.getBody()->accept(*this);
        decreaseIndent();
        decreaseIndent();
    }

    void visit(ReturnStmt &node) override
    {
        printIndent();
        std::cout << "ReturnStmt "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        if (node.getReturnValue())
        {
            increaseIndent();
            node.getReturnValue()->accept(*this);
            decreaseIndent();
        }
    }

    void visit(CompoundStmt &node) override
    {
        printIndent();
        std::cout << "CompoundStmt (" << node.getStatements().size() << " statements) "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        for (const auto &stmt : node.getStatements())
        {
            stmt->accept(*this);
        }
        decreaseIndent();
    }

    // ========================================================================
    // Declaration Visitors
    // ========================================================================

    void visit(VarDecl &node) override
    {
        printIndent();
        std::cout << "VarDecl(" << node.getType() << " " << node.getName() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        if (node.getInitializer())
        {
            increaseIndent();
            printIndent();
            std::cout << "Initializer:" << std::endl;
            increaseIndent();
            node.getInitializer()->accept(*this);
            decreaseIndent();
            decreaseIndent();
        }
    }

    void visit(TypeDecl &node) override
    {
        printIndent();
        std::cout << "TypeDecl(typedef " << node.getUnderlyingType() << " " << node.getName() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;
    }

    void visit(StructDecl &node) override
    {
        printIndent();
        std::cout << "StructDecl(" << node.getName() << ", " << node.getFields().size() << " fields) "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        for (const auto &field : node.getFields())
        {
            field->accept(*this);
        }
        decreaseIndent();
    }

    void visit(FunctionDecl &node) override
    {
        printIndent();
        std::cout << "FunctionDecl(" << node.getReturnType() << " " << node.getName() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;

        increaseIndent();
        printIndent();
        std::cout << "Parameters (" << node.getParameters().size() << "):" << std::endl;
        increaseIndent();
        for (const auto &param : node.getParameters())
        {
            param->accept(*this);
        }
        decreaseIndent();

        if (node.getBody())
        {
            printIndent();
            std::cout << "Body:" << std::endl;
            increaseIndent();
            node.getBody()->accept(*this);
            decreaseIndent();
        }
        else
        {
            printIndent();
            std::cout << "(Forward declaration - no body)" << std::endl;
        }
        decreaseIndent();
    }

    void visit(ParameterDecl &node) override
    {
        printIndent();
        std::cout << "ParameterDecl(" << node.getType() << " " << node.getName() << ") "
                  << "[" << node.getLine() << ":" << node.getColumn() << "]" << std::endl;
    }
};

/*
 * Example Usage:
 * --------------
 *
 * // Create a simple AST: int x = 5;
 * auto init = std::make_unique<LiteralExpr>("5", LiteralExpr::LiteralType::INTEGER,
 *                                            SourceLocation("test.c", 1, 5));
 * auto varDecl = std::make_unique<VarDecl>("x", "int", std::move(init),
 *                                          SourceLocation("test.c", 1, 1));
 *
 * // Print the AST
 * ASTPrinter printer;
 * varDecl->accept(printer);
 *
 * Output:
 *   VarDecl(int x) [1:1]
 *     Initializer:
 *       LiteralExpr(INT: 5) [1:5]
 */
