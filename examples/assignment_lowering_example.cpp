#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// Assignment Lowering Example
// ============================================================================
// Demonstrates how assignment expressions are lowered to IR instructions
//
// Acceptance Criteria:
// 1. x = expr → evaluate expr to temp, then MOVE x, temp
// 2. arr[i] = expr → calculate address, then STORE
// 3. *ptr = expr → STORE ptr, expr

int main() {
    std::cout << "=== Assignment Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple Variable Assignment (x = 42)
    // ========================================================================
    std::cout << "Example 1: Simple Variable Assignment" << std::endl;
    std::cout << "AST: x = 42" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 42" << std::endl;
    std::cout << "  x_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        auto literal = std::make_unique<LiteralExpr>(
            "42",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );
        auto target = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(literal),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 2: Assignment with Expression (x = a + b)
    // ========================================================================
    std::cout << "Example 2: Assignment with Expression" << std::endl;
    std::cout << "AST: x = a + b" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add a_0, b_0" << std::endl;
    std::cout << "  x_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        auto a = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto b = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(a),
            "+",
            std::move(b),
            SourceLocation()
        );
        auto target = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(add),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 3: Array Element Assignment (arr[i] = value)
    // ========================================================================
    std::cout << "Example 3: Array Element Assignment" << std::endl;
    std::cout << "AST: arr[i] = 10" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 10" << std::endl;
    std::cout << "  t1 = add arr_0, i_0" << std::endl;
    std::cout << "  store t0, t1" << std::endl;
    std::cout << std::endl;

    {
        auto value = std::make_unique<LiteralExpr>(
            "10",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto index = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(index),
            SourceLocation()
        );
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(arrayAccess),
            std::move(value),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 4: Array Assignment with Expression (arr[i + 1] = x * 2)
    // ========================================================================
    std::cout << "Example 4: Array Assignment with Complex Expression" << std::endl;
    std::cout << "AST: arr[i + 1] = x * 2" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 2" << std::endl;
    std::cout << "  t1 = mul x_0, t0" << std::endl;
    std::cout << "  t2 = move 1" << std::endl;
    std::cout << "  t3 = add i_0, t2" << std::endl;
    std::cout << "  t4 = add arr_0, t3" << std::endl;
    std::cout << "  store t1, t4" << std::endl;
    std::cout << std::endl;

    {
        // x * 2
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto two = std::make_unique<LiteralExpr>(
            "2",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );
        auto multiply = std::make_unique<BinaryExpr>(
            std::move(x),
            "*",
            std::move(two),
            SourceLocation()
        );

        // i + 1
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto one = std::make_unique<LiteralExpr>(
            "1",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );
        auto indexExpr = std::make_unique<BinaryExpr>(
            std::move(i),
            "+",
            std::move(one),
            SourceLocation()
        );

        // arr[i + 1]
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(indexExpr),
            SourceLocation()
        );

        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(arrayAccess),
            std::move(multiply),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 5: Pointer Dereference Assignment (*ptr = value)
    // ========================================================================
    std::cout << "Example 5: Pointer Dereference Assignment" << std::endl;
    std::cout << "AST: *ptr = 100" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 100" << std::endl;
    std::cout << "  store t0, ptr_0" << std::endl;
    std::cout << std::endl;

    {
        auto value = std::make_unique<LiteralExpr>(
            "100",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,  // prefix
            SourceLocation()
        );
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(deref),
            std::move(value),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 6: Pointer Assignment with Expression (*ptr = a + b)
    // ========================================================================
    std::cout << "Example 6: Pointer Assignment with Expression" << std::endl;
    std::cout << "AST: *ptr = a + b" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add a_0, b_0" << std::endl;
    std::cout << "  store t0, ptr_0" << std::endl;
    std::cout << std::endl;

    {
        auto a = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto b = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(a),
            "+",
            std::move(b),
            SourceLocation()
        );
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,
            SourceLocation()
        );
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(deref),
            std::move(add),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 7: Array Element Read (x = arr[i])
    // ========================================================================
    std::cout << "Example 7: Array Element Read" << std::endl;
    std::cout << "AST: x = arr[i]" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add arr_0, i_0" << std::endl;
    std::cout << "  t1 = load t0" << std::endl;
    std::cout << "  x_0 = move t1" << std::endl;
    std::cout << std::endl;

    {
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto index = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(index),
            SourceLocation()
        );
        auto target = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(arrayAccess),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 8: Pointer Dereference Read (x = *ptr)
    // ========================================================================
    std::cout << "Example 8: Pointer Dereference Read" << std::endl;
    std::cout << "AST: x = *ptr" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = load ptr_0" << std::endl;
    std::cout << "  x_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,
            SourceLocation()
        );
        auto target = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(deref),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== All Assignment Examples Complete ===" << std::endl;

    return 0;
}
