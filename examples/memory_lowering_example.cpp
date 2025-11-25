#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// Memory Access Lowering Example
// ============================================================================
// Demonstrates how memory access operations are lowered to IR
//
// Acceptance Criteria:
// *ptr → LOAD temp, ptr
// *ptr = val → STORE ptr, val
// arr[i] → calculate offset, then LOAD

int main() {
    std::cout << "=== Memory Access Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Pointer Dereference for Reading
    // ========================================================================
    std::cout << "Example 1: Pointer Dereference Read" << std::endl;
    std::cout << "AST: result = *ptr;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = load ptr_0" << std::endl;
    std::cout << "  result_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        // *ptr
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,  // prefix operator
            SourceLocation()
        );

        // result = *ptr
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
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
        codegen.reset();
    }

    // ========================================================================
    // Example 2: Pointer Dereference for Writing
    // ========================================================================
    std::cout << "Example 2: Pointer Dereference Write" << std::endl;
    std::cout << "AST: *ptr = value;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  store value_0, ptr_0" << std::endl;
    std::cout << std::endl;

    {
        // *ptr (target)
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,  // prefix operator
            SourceLocation()
        );

        // value (source)
        auto value = std::make_unique<IdentifierExpr>("value", SourceLocation());

        // *ptr = value
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
        codegen.reset();
    }

    // ========================================================================
    // Example 3: Array Access for Reading
    // ========================================================================
    std::cout << "Example 3: Array Access Read" << std::endl;
    std::cout << "AST: result = arr[i];" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add arr_0, i_0" << std::endl;
    std::cout << "  t1 = load t0" << std::endl;
    std::cout << "  result_0 = move t1" << std::endl;
    std::cout << std::endl;

    {
        // arr[i]
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(i),
            SourceLocation()
        );

        // result = arr[i]
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
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
        codegen.reset();
    }

    // ========================================================================
    // Example 4: Array Access for Writing
    // ========================================================================
    std::cout << "Example 4: Array Access Write" << std::endl;
    std::cout << "AST: arr[i] = value;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add arr_0, i_0" << std::endl;
    std::cout << "  store value_0, t0" << std::endl;
    std::cout << std::endl;

    {
        // arr[i] (target)
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(i),
            SourceLocation()
        );

        // value (source)
        auto value = std::make_unique<IdentifierExpr>("value", SourceLocation());

        // arr[i] = value
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
        codegen.reset();
    }

    // ========================================================================
    // Example 5: Array Access with Expression Index
    // ========================================================================
    std::cout << "Example 5: Array Access with Expression Index" << std::endl;
    std::cout << "AST: result = arr[i + 1];" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 1" << std::endl;
    std::cout << "  t1 = add i_0, t0" << std::endl;
    std::cout << "  t2 = add arr_0, t1" << std::endl;
    std::cout << "  t3 = load t2" << std::endl;
    std::cout << "  result_0 = move t3" << std::endl;
    std::cout << std::endl;

    {
        // Index: i + 1
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
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

        // result = arr[i + 1]
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
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
        codegen.reset();
    }

    // ========================================================================
    // Example 6: Nested Pointer Dereference
    // ========================================================================
    std::cout << "Example 6: Nested Pointer Dereference" << std::endl;
    std::cout << "AST: result = **ptr;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = load ptr_0" << std::endl;
    std::cout << "  t1 = load t0" << std::endl;
    std::cout << "  result_0 = move t1" << std::endl;
    std::cout << std::endl;

    {
        // **ptr
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref1 = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,  // prefix operator
            SourceLocation()
        );
        auto deref2 = std::make_unique<UnaryExpr>(
            "*",
            std::move(deref1),
            true,  // prefix operator
            SourceLocation()
        );

        // result = **ptr
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(deref2),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    // ========================================================================
    // Example 7: Pointer Dereference in Expression
    // ========================================================================
    std::cout << "Example 7: Pointer Dereference in Expression" << std::endl;
    std::cout << "AST: result = *ptr + 10;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = load ptr_0" << std::endl;
    std::cout << "  t1 = move 10" << std::endl;
    std::cout << "  t2 = add t0, t1" << std::endl;
    std::cout << "  result_0 = move t2" << std::endl;
    std::cout << std::endl;

    {
        // *ptr
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,  // prefix operator
            SourceLocation()
        );

        // *ptr + 10
        auto ten = std::make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(deref),
            "+",
            std::move(ten),
            SourceLocation()
        );

        // result = *ptr + 10
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
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
        codegen.reset();
    }

    // ========================================================================
    // Example 8: Array of Pointers
    // ========================================================================
    std::cout << "Example 8: Array of Pointers Dereference" << std::endl;
    std::cout << "AST: result = *arr[i];" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add arr_0, i_0" << std::endl;
    std::cout << "  t1 = load t0" << std::endl;
    std::cout << "  t2 = load t1" << std::endl;
    std::cout << "  result_0 = move t2" << std::endl;
    std::cout << std::endl;

    {
        // arr[i]
        auto arr = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess = std::make_unique<ArrayAccessExpr>(
            std::move(arr),
            std::move(i),
            SourceLocation()
        );

        // *arr[i]
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(arrayAccess),
            true,  // prefix operator
            SourceLocation()
        );

        // result = *arr[i]
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
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
        codegen.reset();
    }

    // ========================================================================
    // Example 9: Complex Memory Access Pattern
    // ========================================================================
    std::cout << "Example 9: Complex Memory Access" << std::endl;
    std::cout << "AST: arr[i] = *ptr + arr[j];" << std::endl;
    std::cout << std::endl;

    {
        // *ptr
        auto ptr = std::make_unique<IdentifierExpr>("ptr", SourceLocation());
        auto deref = std::make_unique<UnaryExpr>(
            "*",
            std::move(ptr),
            true,  // prefix operator
            SourceLocation()
        );

        // arr[j]
        auto arr2 = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto j = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto arrayAccess2 = std::make_unique<ArrayAccessExpr>(
            std::move(arr2),
            std::move(j),
            SourceLocation()
        );

        // *ptr + arr[j]
        auto add = std::make_unique<BinaryExpr>(
            std::move(deref),
            "+",
            std::move(arrayAccess2),
            SourceLocation()
        );

        // arr[i] (target)
        auto arr1 = std::make_unique<IdentifierExpr>("arr", SourceLocation());
        auto i = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto arrayAccess1 = std::make_unique<ArrayAccessExpr>(
            std::move(arr1),
            std::move(i),
            SourceLocation()
        );

        // arr[i] = *ptr + arr[j]
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(arrayAccess1),
            std::move(add),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(assignment.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
        codegen.reset();
    }

    std::cout << "=== All Memory Access Examples Complete ===" << std::endl;

    return 0;
}
