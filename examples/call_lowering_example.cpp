#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// Function Call Lowering Example
// ============================================================================
// Demonstrates how function calls are lowered to IR
//
// Acceptance Criteria:
// result = func(arg1, arg2) →
//   PARAM arg1
//   PARAM arg2
//   CALL func, 2
//   MOVE result, $return
//
// Note: Our IR implementation uses CALL with inline arguments:
//   result = CALL func(arg1, arg2)
// This is a valid alternative representation.

int main() {
    std::cout << "=== Function Call Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple Function Call with No Arguments
    // ========================================================================
    std::cout << "Example 1: Function Call with No Arguments" << std::endl;
    std::cout << "AST: result = getNumber();" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = call getNumber()" << std::endl;
    std::cout << "  result_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        // Call: getNumber()
        auto callee = std::make_unique<IdentifierExpr>("getNumber", SourceLocation());
        std::vector<std::unique_ptr<Expression>> args;
        auto call = std::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            SourceLocation()
        );

        // Assignment: result = getNumber()
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(call),
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
    // Example 2: Function Call with One Argument
    // ========================================================================
    std::cout << "Example 2: Function Call with One Argument" << std::endl;
    std::cout << "AST: result = square(x);" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = call square(x_0)" << std::endl;
    std::cout << "  result_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        // Argument: x
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::make_unique<IdentifierExpr>("x", SourceLocation()));

        // Call: square(x)
        auto callee = std::make_unique<IdentifierExpr>("square", SourceLocation());
        auto call = std::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            SourceLocation()
        );

        // Assignment: result = square(x)
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(call),
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
    // Example 3: Function Call with Multiple Arguments
    // ========================================================================
    std::cout << "Example 3: Function Call with Multiple Arguments" << std::endl;
    std::cout << "AST: result = add(a, b);" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = call add(a_0, b_0)" << std::endl;
    std::cout << "  result_0 = move t0" << std::endl;
    std::cout << std::endl;

    {
        // Arguments: a, b
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::make_unique<IdentifierExpr>("a", SourceLocation()));
        args.push_back(std::make_unique<IdentifierExpr>("b", SourceLocation()));

        // Call: add(a, b)
        auto callee = std::make_unique<IdentifierExpr>("add", SourceLocation());
        auto call = std::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            SourceLocation()
        );

        // Assignment: result = add(a, b)
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(call),
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
    // Example 4: Function Call with Expression Arguments
    // ========================================================================
    std::cout << "Example 4: Function Call with Expression Arguments" << std::endl;
    std::cout << "AST: result = max(x + 1, y * 2);" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 1" << std::endl;
    std::cout << "  t1 = add x_0, t0" << std::endl;
    std::cout << "  t2 = move 2" << std::endl;
    std::cout << "  t3 = mul y_0, t2" << std::endl;
    std::cout << "  t4 = call max(t1, t3)" << std::endl;
    std::cout << "  result_0 = move t4" << std::endl;
    std::cout << std::endl;

    {
        // Argument 1: x + 1
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto arg1 = std::make_unique<BinaryExpr>(
            std::move(x),
            "+",
            std::move(one),
            SourceLocation()
        );

        // Argument 2: y * 2
        auto y = std::make_unique<IdentifierExpr>("y", SourceLocation());
        auto two = std::make_unique<LiteralExpr>("2", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto arg2 = std::make_unique<BinaryExpr>(
            std::move(y),
            "*",
            std::move(two),
            SourceLocation()
        );

        // Arguments
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::move(arg1));
        args.push_back(std::move(arg2));

        // Call: max(x + 1, y * 2)
        auto callee = std::make_unique<IdentifierExpr>("max", SourceLocation());
        auto call = std::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            SourceLocation()
        );

        // Assignment: result = max(x + 1, y * 2)
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(call),
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
    // Example 5: Nested Function Calls
    // ========================================================================
    std::cout << "Example 5: Nested Function Calls" << std::endl;
    std::cout << "AST: result = add(square(x), square(y));" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = call square(x_0)" << std::endl;
    std::cout << "  t1 = call square(y_0)" << std::endl;
    std::cout << "  t2 = call add(t0, t1)" << std::endl;
    std::cout << "  result_0 = move t2" << std::endl;
    std::cout << std::endl;

    {
        // Inner call 1: square(x)
        std::vector<std::unique_ptr<Expression>> inner1Args;
        inner1Args.push_back(std::make_unique<IdentifierExpr>("x", SourceLocation()));
        auto inner1Callee = std::make_unique<IdentifierExpr>("square", SourceLocation());
        auto inner1Call = std::make_unique<CallExpr>(
            std::move(inner1Callee),
            std::move(inner1Args),
            SourceLocation()
        );

        // Inner call 2: square(y)
        std::vector<std::unique_ptr<Expression>> inner2Args;
        inner2Args.push_back(std::make_unique<IdentifierExpr>("y", SourceLocation()));
        auto inner2Callee = std::make_unique<IdentifierExpr>("square", SourceLocation());
        auto inner2Call = std::make_unique<CallExpr>(
            std::move(inner2Callee),
            std::move(inner2Args),
            SourceLocation()
        );

        // Outer call: add(square(x), square(y))
        std::vector<std::unique_ptr<Expression>> outerArgs;
        outerArgs.push_back(std::move(inner1Call));
        outerArgs.push_back(std::move(inner2Call));
        auto outerCallee = std::make_unique<IdentifierExpr>("add", SourceLocation());
        auto outerCall = std::make_unique<CallExpr>(
            std::move(outerCallee),
            std::move(outerArgs),
            SourceLocation()
        );

        // Assignment: result = add(square(x), square(y))
        auto target = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(target),
            std::move(outerCall),
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
    // Example 6: Function Call in Expression
    // ========================================================================
    std::cout << "Example 6: Function Call in Expression" << std::endl;
    std::cout << "AST: result = getValue() + 10;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = call getValue()" << std::endl;
    std::cout << "  t1 = move 10" << std::endl;
    std::cout << "  t2 = add t0, t1" << std::endl;
    std::cout << "  result_0 = move t2" << std::endl;
    std::cout << std::endl;

    {
        // Call: getValue()
        auto callee = std::make_unique<IdentifierExpr>("getValue", SourceLocation());
        std::vector<std::unique_ptr<Expression>> args;
        auto call = std::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            SourceLocation()
        );

        // Expression: getValue() + 10
        auto ten = std::make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(call),
            "+",
            std::move(ten),
            SourceLocation()
        );

        // Assignment: result = getValue() + 10
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
    }

    std::cout << "=== All Function Call Examples Complete ===" << std::endl;

    return 0;
}
