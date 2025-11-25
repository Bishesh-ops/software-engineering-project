#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// For Loop Lowering Example
// ============================================================================
// Demonstrates how for loops are lowered to IR with init, condition, update
//
// Acceptance Criteria:
// for (init; cond; update) body →
//   init
//   L_start:
//   eval cond → tc
//   JUMP_IF_FALSE tc, L_end
//   body
//   update
//   JUMP L_start
//   L_end:

int main() {
    std::cout << "=== For Loop Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Classic For Loop (for (i = 0; i < 10; i = i + 1))
    // ========================================================================
    std::cout << "Example 1: Classic For Loop" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  for (i = 0; i < 10; i = i + 1)" << std::endl;
    std::cout << "    sum = sum + i;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 0" << std::endl;
    std::cout << "  i_0 = move t0" << std::endl;
    std::cout << "  for_start_0:" << std::endl;
    std::cout << "  t1 = lt i_0, 10" << std::endl;
    std::cout << "  jump_if_false t1, for_end_1" << std::endl;
    std::cout << "  t2 = add sum_0, i_0" << std::endl;
    std::cout << "  sum_0 = move t2" << std::endl;
    std::cout << "  t3 = add i_0, 1" << std::endl;
    std::cout << "  i_0 = move t3" << std::endl;
    std::cout << "  jump for_start_0" << std::endl;
    std::cout << "  for_end_1:" << std::endl;
    std::cout << std::endl;

    {
        // Init: i = 0
        auto zero = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto i1 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto init = std::make_unique<AssignmentExpr>(
            std::move(i1),
            std::move(zero),
            SourceLocation()
        );
        auto initStmt = std::make_unique<ExpressionStmt>(std::move(init), SourceLocation());

        // Condition: i < 10
        auto i2 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto ten = std::make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(i2),
            "<",
            std::move(ten),
            SourceLocation()
        );

        // Update: i = i + 1
        auto i3 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(i3),
            "+",
            std::move(one),
            SourceLocation()
        );
        auto i4 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto update = std::make_unique<AssignmentExpr>(
            std::move(i4),
            std::move(add),
            SourceLocation()
        );

        // Body: sum = sum + i
        auto sum1 = std::make_unique<IdentifierExpr>("sum", SourceLocation());
        auto i5 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto bodyAdd = std::make_unique<BinaryExpr>(
            std::move(sum1),
            "+",
            std::move(i5),
            SourceLocation()
        );
        auto sum2 = std::make_unique<IdentifierExpr>("sum", SourceLocation());
        auto bodyAssign = std::make_unique<AssignmentExpr>(
            std::move(sum2),
            std::move(bodyAdd),
            SourceLocation()
        );
        auto body = std::make_unique<ExpressionStmt>(std::move(bodyAssign), SourceLocation());

        // For loop
        auto forStmt = std::make_unique<ForStmt>(
            std::move(initStmt),
            std::move(condition),
            std::move(update),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(forStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 2: For Loop with Multiple Statements in Body
    // ========================================================================
    std::cout << "Example 2: For Loop with Multiple Statements" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  for (n = 5; n > 0; n = n - 1) {" << std::endl;
    std::cout << "    product = product * n;" << std::endl;
    std::cout << "    count = count + 1;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;

    {
        // Init: n = 5
        auto five = std::make_unique<LiteralExpr>("5", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto n1 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto init = std::make_unique<AssignmentExpr>(
            std::move(n1),
            std::move(five),
            SourceLocation()
        );
        auto initStmt = std::make_unique<ExpressionStmt>(std::move(init), SourceLocation());

        // Condition: n > 0
        auto n2 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto zero = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(n2),
            ">",
            std::move(zero),
            SourceLocation()
        );

        // Update: n = n - 1
        auto n3 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto sub = std::make_unique<BinaryExpr>(
            std::move(n3),
            "-",
            std::move(one),
            SourceLocation()
        );
        auto n4 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto update = std::make_unique<AssignmentExpr>(
            std::move(n4),
            std::move(sub),
            SourceLocation()
        );

        // Body statement 1: product = product * n
        auto product1 = std::make_unique<IdentifierExpr>("product", SourceLocation());
        auto n5 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto mul = std::make_unique<BinaryExpr>(
            std::move(product1),
            "*",
            std::move(n5),
            SourceLocation()
        );
        auto product2 = std::make_unique<IdentifierExpr>("product", SourceLocation());
        auto assign1 = std::make_unique<AssignmentExpr>(
            std::move(product2),
            std::move(mul),
            SourceLocation()
        );
        auto stmt1 = std::make_unique<ExpressionStmt>(std::move(assign1), SourceLocation());

        // Body statement 2: count = count + 1
        auto count1 = std::make_unique<IdentifierExpr>("count", SourceLocation());
        auto one2 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(count1),
            "+",
            std::move(one2),
            SourceLocation()
        );
        auto count2 = std::make_unique<IdentifierExpr>("count", SourceLocation());
        auto assign2 = std::make_unique<AssignmentExpr>(
            std::move(count2),
            std::move(add),
            SourceLocation()
        );
        auto stmt2 = std::make_unique<ExpressionStmt>(std::move(assign2), SourceLocation());

        // Compound body
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(stmt1));
        statements.push_back(std::move(stmt2));
        auto body = std::make_unique<CompoundStmt>(std::move(statements), SourceLocation());

        // For loop
        auto forStmt = std::make_unique<ForStmt>(
            std::move(initStmt),
            std::move(condition),
            std::move(update),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(forStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 3: For Loop with Complex Condition and Update
    // ========================================================================
    std::cout << "Example 3: For Loop with Complex Expressions" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  for (x = 1; x * 2 < 100; x = x + 5)" << std::endl;
    std::cout << "    result = x;" << std::endl;
    std::cout << std::endl;

    {
        // Init: x = 1
        auto one1 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto x1 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto init = std::make_unique<AssignmentExpr>(
            std::move(x1),
            std::move(one1),
            SourceLocation()
        );
        auto initStmt = std::make_unique<ExpressionStmt>(std::move(init), SourceLocation());

        // Condition: x * 2 < 100
        auto x2 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto two = std::make_unique<LiteralExpr>("2", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto mul = std::make_unique<BinaryExpr>(
            std::move(x2),
            "*",
            std::move(two),
            SourceLocation()
        );
        auto hundred = std::make_unique<LiteralExpr>("100", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(mul),
            "<",
            std::move(hundred),
            SourceLocation()
        );

        // Update: x = x + 5
        auto x3 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto five = std::make_unique<LiteralExpr>("5", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(x3),
            "+",
            std::move(five),
            SourceLocation()
        );
        auto x4 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto update = std::make_unique<AssignmentExpr>(
            std::move(x4),
            std::move(add),
            SourceLocation()
        );

        // Body: result = x
        auto x5 = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto result = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto bodyAssign = std::make_unique<AssignmentExpr>(
            std::move(result),
            std::move(x5),
            SourceLocation()
        );
        auto body = std::make_unique<ExpressionStmt>(std::move(bodyAssign), SourceLocation());

        // For loop
        auto forStmt = std::make_unique<ForStmt>(
            std::move(initStmt),
            std::move(condition),
            std::move(update),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(forStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 4: Nested For Loops
    // ========================================================================
    std::cout << "Example 4: Nested For Loops" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  for (i = 0; i < 2; i = i + 1)" << std::endl;
    std::cout << "    for (j = 0; j < 3; j = j + 1)" << std::endl;
    std::cout << "      sum = sum + 1;" << std::endl;
    std::cout << std::endl;

    {
        // Inner loop: for (j = 0; j < 3; j = j + 1) sum = sum + 1
        auto zero1 = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto j1 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto innerInit = std::make_unique<AssignmentExpr>(
            std::move(j1),
            std::move(zero1),
            SourceLocation()
        );
        auto innerInitStmt = std::make_unique<ExpressionStmt>(std::move(innerInit), SourceLocation());

        auto j2 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto three = std::make_unique<LiteralExpr>("3", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto innerCondition = std::make_unique<BinaryExpr>(
            std::move(j2),
            "<",
            std::move(three),
            SourceLocation()
        );

        auto j3 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto one1 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto innerAdd = std::make_unique<BinaryExpr>(
            std::move(j3),
            "+",
            std::move(one1),
            SourceLocation()
        );
        auto j4 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto innerUpdate = std::make_unique<AssignmentExpr>(
            std::move(j4),
            std::move(innerAdd),
            SourceLocation()
        );

        auto sum1 = std::make_unique<IdentifierExpr>("sum", SourceLocation());
        auto one2 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto innerBodyAdd = std::make_unique<BinaryExpr>(
            std::move(sum1),
            "+",
            std::move(one2),
            SourceLocation()
        );
        auto sum2 = std::make_unique<IdentifierExpr>("sum", SourceLocation());
        auto innerBodyAssign = std::make_unique<AssignmentExpr>(
            std::move(sum2),
            std::move(innerBodyAdd),
            SourceLocation()
        );
        auto innerBody = std::make_unique<ExpressionStmt>(std::move(innerBodyAssign), SourceLocation());

        auto innerFor = std::make_unique<ForStmt>(
            std::move(innerInitStmt),
            std::move(innerCondition),
            std::move(innerUpdate),
            std::move(innerBody),
            SourceLocation()
        );

        // Outer loop: for (i = 0; i < 2; i = i + 1)
        auto zero2 = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto i1 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto outerInit = std::make_unique<AssignmentExpr>(
            std::move(i1),
            std::move(zero2),
            SourceLocation()
        );
        auto outerInitStmt = std::make_unique<ExpressionStmt>(std::move(outerInit), SourceLocation());

        auto i2 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto two = std::make_unique<LiteralExpr>("2", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto outerCondition = std::make_unique<BinaryExpr>(
            std::move(i2),
            "<",
            std::move(two),
            SourceLocation()
        );

        auto i3 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto one3 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto outerAdd = std::make_unique<BinaryExpr>(
            std::move(i3),
            "+",
            std::move(one3),
            SourceLocation()
        );
        auto i4 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto outerUpdate = std::make_unique<AssignmentExpr>(
            std::move(i4),
            std::move(outerAdd),
            SourceLocation()
        );

        auto outerFor = std::make_unique<ForStmt>(
            std::move(outerInitStmt),
            std::move(outerCondition),
            std::move(outerUpdate),
            std::move(innerFor),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(outerFor.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== All For Loop Examples Complete ===" << std::endl;

    return 0;
}
