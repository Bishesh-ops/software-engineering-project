#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// IR Printer Example
// ============================================================================
// Demonstrates the IR printer functionality with readable output format
//
// Acceptance Criteria:
// - One instruction per line
// - Clear format
// Example:
//     t0 = b * c
//     t1 = a + t0
//     L0:
//     JUMP_IF_FALSE t1, L1
//     RETURN t1
//     L1:

int main() {
    std::cout << "=== IR Printer Debug Output Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple Arithmetic Expression
    // ========================================================================
    std::cout << "Example 1: Simple Arithmetic Expression" << std::endl;
    std::cout << "Source: result = a + b * c;" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

    {
        // b * c
        auto b = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto c = std::make_unique<IdentifierExpr>("c", SourceLocation());
        auto mul = std::make_unique<BinaryExpr>(
            std::move(b),
            "*",
            std::move(c),
            SourceLocation()
        );

        // a + (b * c)
        auto a = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(a),
            "+",
            std::move(mul),
            SourceLocation()
        );

        // result = a + b * c
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(add),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    // ========================================================================
    // Example 2: If Statement with Control Flow
    // ========================================================================
    std::cout << "Example 2: If Statement with Control Flow" << std::endl;
    std::cout << "Source:" << std::endl;
    std::cout << "  if (x > 0)" << std::endl;
    std::cout << "    return x;" << std::endl;
    std::cout << "  else" << std::endl;
    std::cout << "    return 0;" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

    {
        // Condition: x > 0
        auto x1 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto zero1 = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(x1),
            ">",
            std::move(zero1),
            SourceLocation()
        );

        // Then: return x
        auto x2 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto thenReturn = std::make_unique<ReturnStmt>(std::move(x2), SourceLocation());

        // Else: return 0
        auto zero2 = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto elseReturn = std::make_unique<ReturnStmt>(std::move(zero2), SourceLocation());

        // If statement
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition),
            std::move(thenReturn),
            std::move(elseReturn),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(ifStmt.get());

        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    // ========================================================================
    // Example 3: While Loop
    // ========================================================================
    std::cout << "Example 3: While Loop" << std::endl;
    std::cout << "Source:" << std::endl;
    std::cout << "  while (i < 10)" << std::endl;
    std::cout << "    i = i + 1;" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

    {
        // Condition: i < 10
        auto i1 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto ten = std::make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(i1),
            "<",
            std::move(ten),
            SourceLocation()
        );

        // Body: i = i + 1
        auto i2 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(i2),
            "+",
            std::move(one),
            SourceLocation()
        );
        auto i3 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(i3),
            std::move(add),
            SourceLocation()
        );
        auto body = std::make_unique<ExpressionStmt>(std::move(assignment), SourceLocation());

        // While statement
        auto whileStmt = std::make_unique<WhileStmt>(
            std::move(condition),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(whileStmt.get());

        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    // ========================================================================
    // Example 4: Function Call
    // ========================================================================
    std::cout << "Example 4: Function Call" << std::endl;
    std::cout << "Source: result = add(x, y);" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

    {
        // Arguments
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::make_unique<IdentifierExpr>("x", SourceLocation()));
        args.push_back(std::make_unique<IdentifierExpr>("y", SourceLocation()));

        // Call: add(x, y)
        auto callee = std::make_unique<IdentifierExpr>("add", SourceLocation());
        auto call = std::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            SourceLocation()
        );

        // result = add(x, y)
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(call),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    // ========================================================================
    // Example 5: Memory Operations
    // ========================================================================
    std::cout << "Example 5: Memory Operations" << std::endl;
    std::cout << "Source: result = *ptr + arr[i];" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

    {
        // *ptr
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,
            SourceLocation()
        );

        // arr[i]
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(i),
            SourceLocation()
        );

        // *ptr + arr[i]
        auto add = std::make_unique<BinaryExpr>(
            std::move(deref),
            "+",
            std::move(arrayAccess),
            SourceLocation()
        );

        // result = *ptr + arr[i]
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(add),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    // ========================================================================
    // Example 6: Complete Function (showing full IR format)
    // ========================================================================
    std::cout << "Example 6: Complete Function IR" << std::endl;
    std::cout << "Source:" << std::endl;
    std::cout << "  int max(int a, int b) {" << std::endl;
    std::cout << "    if (a > b)" << std::endl;
    std::cout << "      return a;" << std::endl;
    std::cout << "    else" << std::endl;
    std::cout << "      return b;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

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

        // Function body
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

        std::cout << irFunc->toString() << std::endl;
    }

    // ========================================================================
    // Example 7: Complex Function with Multiple Blocks
    // ========================================================================
    std::cout << "Example 7: Complex Function with Loop" << std::endl;
    std::cout << "Source:" << std::endl;
    std::cout << "  int factorial(int n) {" << std::endl;
    std::cout << "    result = 1;" << std::endl;
    std::cout << "    while (n > 0) {" << std::endl;
    std::cout << "      result = result * n;" << std::endl;
    std::cout << "      n = n - 1;" << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "    return result;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "IR Output:" << std::endl;

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

        std::cout << irFunc->toString() << std::endl;
    }

    std::cout << "=== All IR Printer Examples Complete ===" << std::endl;
    std::cout << std::endl;
    std::cout << "Key Features Demonstrated:" << std::endl;
    std::cout << "  ✓ One instruction per line" << std::endl;
    std::cout << "  ✓ Clear, readable format" << std::endl;
    std::cout << "  ✓ Labels followed by colon" << std::endl;
    std::cout << "  ✓ Instructions indented with 2 spaces" << std::endl;
    std::cout << "  ✓ Function signature with parameters" << std::endl;
    std::cout << "  ✓ Basic blocks with control flow" << std::endl;
    std::cout << "  ✓ SSA form with unique variable names" << std::endl;

    return 0;
}
