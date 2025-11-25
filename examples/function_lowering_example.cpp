#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// Function Definition Lowering Example
// ============================================================================
// Demonstrates how function definitions are lowered to IR
//
// Acceptance Criteria:
// - Function entry label
// - Parameter setup
// - Body lowering
// - Return handling

int main() {
    std::cout << "=== Function Definition Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple Function with No Parameters and No Return
    // ========================================================================
    std::cout << "Example 1: Simple Void Function" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  void doNothing() {" << std::endl;
    std::cout << "    return;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  function void doNothing() {" << std::endl;
    std::cout << "    doNothing_entry:" << std::endl;
    std::cout << "      return" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;

    {
        // Body: return;
        auto returnStmt = std::make_unique<ReturnStmt>(nullptr, SourceLocation());
        std::vector<std::unique_ptr<Statement>> bodyStatements;
        bodyStatements.push_back(std::move(returnStmt));
        auto body = std::make_unique<CompoundStmt>(std::move(bodyStatements), SourceLocation());

        // Function: void doNothing()
        std::vector<std::unique_ptr<ParameterDecl>> params;
        auto funcDecl = std::make_unique<FunctionDecl>(
            "doNothing",
            "void",
            std::move(params),
            std::move(body),
            SourceLocation()
        );

        auto irFunc = codegen.generateFunctionIR(funcDecl.get());

        std::cout << "Generated IR:" << std::endl;
        std::cout << irFunc->toString() << std::endl;
    }

    // ========================================================================
    // Example 2: Function with Parameters and Return Value
    // ========================================================================
    std::cout << "Example 2: Function with Parameters" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  int add(int a, int b) {" << std::endl;
    std::cout << "    return a + b;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;

    {
        // Body: return a + b;
        auto a = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto b = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(a),
            "+",
            std::move(b),
            SourceLocation()
        );
        auto returnStmt = std::make_unique<ReturnStmt>(std::move(add), SourceLocation());
        std::vector<std::unique_ptr<Statement>> bodyStatements;
        bodyStatements.push_back(std::move(returnStmt));
        auto body = std::make_unique<CompoundStmt>(std::move(bodyStatements), SourceLocation());

        // Parameters
        std::vector<std::unique_ptr<ParameterDecl>> params;
        params.push_back(std::make_unique<ParameterDecl>("a", "int", SourceLocation()));
        params.push_back(std::make_unique<ParameterDecl>("b", "int", SourceLocation()));

        // Function
        auto funcDecl = std::make_unique<FunctionDecl>(
            "add",
            "int",
            std::move(params),
            std::move(body),
            SourceLocation()
        );

        auto irFunc = codegen.generateFunctionIR(funcDecl.get());

        std::cout << "Generated IR:" << std::endl;
        std::cout << irFunc->toString() << std::endl;
    }

    // ========================================================================
    // Example 3: Function with Local Variables and Multiple Statements
    // ========================================================================
    std::cout << "Example 3: Function with Multiple Statements" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  int square(int x) {" << std::endl;
    std::cout << "    result = x * x;" << std::endl;
    std::cout << "    return result;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;

    {
        // Statement 1: result = x * x
        auto x1 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto x2 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto mul = std::make_unique<BinaryExpr>(
            std::move(x1),
            "*",
            std::move(x2),
            SourceLocation()
        );
        auto result1 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assign = std::make_unique<AssignmentExpr>(
            std::move(result1),
            std::move(mul),
            SourceLocation()
        );
        auto stmt1 = std::make_unique<ExpressionStmt>(std::move(assign), SourceLocation());

        // Statement 2: return result
        auto result2 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto returnStmt = std::make_unique<ReturnStmt>(std::move(result2), SourceLocation());

        std::vector<std::unique_ptr<Statement>> bodyStatements;
        bodyStatements.push_back(std::move(stmt1));
        bodyStatements.push_back(std::move(returnStmt));
        auto body = std::make_unique<CompoundStmt>(std::move(bodyStatements), SourceLocation());

        // Parameter
        std::vector<std::unique_ptr<ParameterDecl>> params;
        params.push_back(std::make_unique<ParameterDecl>("x", "int", SourceLocation()));

        // Function
        auto funcDecl = std::make_unique<FunctionDecl>(
            "square",
            "int",
            std::move(params),
            std::move(body),
            SourceLocation()
        );

        auto irFunc = codegen.generateFunctionIR(funcDecl.get());

        std::cout << "Generated IR:" << std::endl;
        std::cout << irFunc->toString() << std::endl;
    }

    // ========================================================================
    // Example 4: Function with Control Flow
    // ========================================================================
    std::cout << "Example 4: Function with If Statement" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  int max(int a, int b) {" << std::endl;
    std::cout << "    if (a > b)" << std::endl;
    std::cout << "      return a;" << std::endl;
    std::cout << "    else" << std::endl;
    std::cout << "      return b;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;

    {
        // Condition: a > b
        auto a1 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto b1 = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(a1),
            ">",
            std::move(b1),
            SourceLocation()
        );

        // Then: return a
        auto a2 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto thenReturn = std::make_unique<ReturnStmt>(std::move(a2), SourceLocation());

        // Else: return b
        auto b2 = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto elseReturn = std::make_unique<ReturnStmt>(std::move(b2), SourceLocation());

        // If statement
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition),
            std::move(thenReturn),
            std::move(elseReturn),
            SourceLocation()
        );

        std::vector<std::unique_ptr<Statement>> bodyStatements;
        bodyStatements.push_back(std::move(ifStmt));
        auto body = std::make_unique<CompoundStmt>(std::move(bodyStatements), SourceLocation());

        // Parameters
        std::vector<std::unique_ptr<ParameterDecl>> params;
        params.push_back(std::make_unique<ParameterDecl>("a", "int", SourceLocation()));
        params.push_back(std::make_unique<ParameterDecl>("b", "int", SourceLocation()));

        // Function
        auto funcDecl = std::make_unique<FunctionDecl>(
            "max",
            "int",
            std::move(params),
            std::move(body),
            SourceLocation()
        );

        auto irFunc = codegen.generateFunctionIR(funcDecl.get());

        std::cout << "Generated IR:" << std::endl;
        std::cout << irFunc->toString() << std::endl;
    }

    // ========================================================================
    // Example 5: Function with Loop
    // ========================================================================
    std::cout << "Example 5: Function with Loop" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  int factorial(int n) {" << std::endl;
    std::cout << "    result = 1;" << std::endl;
    std::cout << "    while (n > 0) {" << std::endl;
    std::cout << "      result = result * n;" << std::endl;
    std::cout << "      n = n - 1;" << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "    return result;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;

    {
        // Statement 1: result = 1
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto result1 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto init = std::make_unique<AssignmentExpr>(
            std::move(result1),
            std::move(one),
            SourceLocation()
        );
        auto initStmt = std::make_unique<ExpressionStmt>(std::move(init), SourceLocation());

        // While condition: n > 0
        auto n1 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto zero = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(n1),
            ">",
            std::move(zero),
            SourceLocation()
        );

        // While body statement 1: result = result * n
        auto result2 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto n2 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto mul = std::make_unique<BinaryExpr>(
            std::move(result2),
            "*",
            std::move(n2),
            SourceLocation()
        );
        auto result3 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assign1 = std::make_unique<AssignmentExpr>(
            std::move(result3),
            std::move(mul),
            SourceLocation()
        );
        auto whileStmt1 = std::make_unique<ExpressionStmt>(std::move(assign1), SourceLocation());

        // While body statement 2: n = n - 1
        auto n3 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto one2 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto sub = std::make_unique<BinaryExpr>(
            std::move(n3),
            "-",
            std::move(one2),
            SourceLocation()
        );
        auto n4 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto assign2 = std::make_unique<AssignmentExpr>(
            std::move(n4),
            std::move(sub),
            SourceLocation()
        );
        auto whileStmt2 = std::make_unique<ExpressionStmt>(std::move(assign2), SourceLocation());

        // While body
        std::vector<std::unique_ptr<Statement>> whileBody;
        whileBody.push_back(std::move(whileStmt1));
        whileBody.push_back(std::move(whileStmt2));
        auto whileBodyCompound = std::make_unique<CompoundStmt>(std::move(whileBody), SourceLocation());

        // While statement
        auto whileLoop = std::make_unique<WhileStmt>(
            std::move(condition),
            std::move(whileBodyCompound),
            SourceLocation()
        );

        // Return statement
        auto result4 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto returnStmt = std::make_unique<ReturnStmt>(std::move(result4), SourceLocation());

        // Function body
        std::vector<std::unique_ptr<Statement>> bodyStatements;
        bodyStatements.push_back(std::move(initStmt));
        bodyStatements.push_back(std::move(whileLoop));
        bodyStatements.push_back(std::move(returnStmt));
        auto body = std::make_unique<CompoundStmt>(std::move(bodyStatements), SourceLocation());

        // Parameter
        std::vector<std::unique_ptr<ParameterDecl>> params;
        params.push_back(std::make_unique<ParameterDecl>("n", "int", SourceLocation()));

        // Function
        auto funcDecl = std::make_unique<FunctionDecl>(
            "factorial",
            "int",
            std::move(params),
            std::move(body),
            SourceLocation()
        );

        auto irFunc = codegen.generateFunctionIR(funcDecl.get());

        std::cout << "Generated IR:" << std::endl;
        std::cout << irFunc->toString() << std::endl;
    }

    std::cout << "=== All Function Definition Examples Complete ===" << std::endl;

    return 0;
}
