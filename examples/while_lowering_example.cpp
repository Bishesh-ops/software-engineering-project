#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// While Loop Lowering Example
// ============================================================================
// Demonstrates how while loops are lowered to IR with conditional jumps
//
// Acceptance Criteria:
// while (cond) body →
//   L_start:
//   eval cond → tc
//   JUMP_IF_FALSE tc, L_end
//   body
//   JUMP L_start
//   L_end:

int main() {
    std::cout << "=== While Loop Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple While Loop
    // ========================================================================
    std::cout << "Example 1: Simple While Loop" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  while (i < 10)" << std::endl;
    std::cout << "    i = i + 1;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  while_start_0:" << std::endl;
    std::cout << "  t0 = lt i_0, 10" << std::endl;
    std::cout << "  jump_if_false t0, while_end_1" << std::endl;
    std::cout << "  t1 = add i_0, 1" << std::endl;
    std::cout << "  i_0 = move t1" << std::endl;
    std::cout << "  jump while_start_0" << std::endl;
    std::cout << "  while_end_1:" << std::endl;
    std::cout << std::endl;

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

        // While loop
        auto whileStmt = std::make_unique<WhileStmt>(
            std::move(condition),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(whileStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 2: While Loop with Multiple Statements
    // ========================================================================
    std::cout << "Example 2: While Loop with Multiple Statements" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  while (n > 0) {" << std::endl;
    std::cout << "    sum = sum + n;" << std::endl;
    std::cout << "    n = n - 1;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  while_start_0:" << std::endl;
    std::cout << "  t0 = gt n_0, 0" << std::endl;
    std::cout << "  jump_if_false t0, while_end_1" << std::endl;
    std::cout << "  t1 = add sum_0, n_0" << std::endl;
    std::cout << "  sum_0 = move t1" << std::endl;
    std::cout << "  t2 = sub n_0, 1" << std::endl;
    std::cout << "  n_0 = move t2" << std::endl;
    std::cout << "  jump while_start_0" << std::endl;
    std::cout << "  while_end_1:" << std::endl;
    std::cout << std::endl;

    {
        // Condition: n > 0
        auto n1 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto zero = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(n1),
            ">",
            std::move(zero),
            SourceLocation()
        );

        // Statement 1: sum = sum + n
        auto sum1 = std::make_unique<IdentifierExpr>("sum", SourceLocation());
        auto n2 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(sum1),
            "+",
            std::move(n2),
            SourceLocation()
        );
        auto sum2 = std::make_unique<IdentifierExpr>("sum", SourceLocation());
        auto assign1 = std::make_unique<AssignmentExpr>(
            std::move(sum2),
            std::move(add),
            SourceLocation()
        );
        auto stmt1 = std::make_unique<ExpressionStmt>(std::move(assign1), SourceLocation());

        // Statement 2: n = n - 1
        auto n3 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto sub = std::make_unique<BinaryExpr>(
            std::move(n3),
            "-",
            std::move(one),
            SourceLocation()
        );
        auto n4 = std::make_unique<IdentifierExpr>("n", SourceLocation());
        auto assign2 = std::make_unique<AssignmentExpr>(
            std::move(n4),
            std::move(sub),
            SourceLocation()
        );
        auto stmt2 = std::make_unique<ExpressionStmt>(std::move(assign2), SourceLocation());

        // Compound body
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(stmt1));
        statements.push_back(std::move(stmt2));
        auto body = std::make_unique<CompoundStmt>(std::move(statements), SourceLocation());

        // While loop
        auto whileStmt = std::make_unique<WhileStmt>(
            std::move(condition),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(whileStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 3: While Loop with Complex Condition
    // ========================================================================
    std::cout << "Example 3: While Loop with Complex Condition" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  while (a + b < 100)" << std::endl;
    std::cout << "    a = a + 1;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  while_start_0:" << std::endl;
    std::cout << "  t0 = add a_0, b_0" << std::endl;
    std::cout << "  t1 = move 100" << std::endl;
    std::cout << "  t2 = lt t0, t1" << std::endl;
    std::cout << "  jump_if_false t2, while_end_1" << std::endl;
    std::cout << "  t3 = move 1" << std::endl;
    std::cout << "  t4 = add a_0, t3" << std::endl;
    std::cout << "  a_0 = move t4" << std::endl;
    std::cout << "  jump while_start_0" << std::endl;
    std::cout << "  while_end_1:" << std::endl;
    std::cout << std::endl;

    {
        // Condition: (a + b) < 100
        auto a1 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto b1 = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto sum = std::make_unique<BinaryExpr>(
            std::move(a1),
            "+",
            std::move(b1),
            SourceLocation()
        );
        auto hundred = std::make_unique<LiteralExpr>("100", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(sum),
            "<",
            std::move(hundred),
            SourceLocation()
        );

        // Body: a = a + 1
        auto a2 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto add = std::make_unique<BinaryExpr>(
            std::move(a2),
            "+",
            std::move(one),
            SourceLocation()
        );
        auto a3 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(a3),
            std::move(add),
            SourceLocation()
        );
        auto body = std::make_unique<ExpressionStmt>(std::move(assignment), SourceLocation());

        // While loop
        auto whileStmt = std::make_unique<WhileStmt>(
            std::move(condition),
            std::move(body),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(whileStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 4: Nested While Loop
    // ========================================================================
    std::cout << "Example 4: Nested While Loop" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  while (i < 3) {" << std::endl;
    std::cout << "    while (j < 2)" << std::endl;
    std::cout << "      j = j + 1;" << std::endl;
    std::cout << "    i = i + 1;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  while_start_0:" << std::endl;
    std::cout << "  t0 = lt i_0, 3" << std::endl;
    std::cout << "  jump_if_false t0, while_end_1" << std::endl;
    std::cout << "  while_start_2:" << std::endl;
    std::cout << "  t1 = lt j_0, 2" << std::endl;
    std::cout << "  jump_if_false t1, while_end_3" << std::endl;
    std::cout << "  t2 = add j_0, 1" << std::endl;
    std::cout << "  j_0 = move t2" << std::endl;
    std::cout << "  jump while_start_2" << std::endl;
    std::cout << "  while_end_3:" << std::endl;
    std::cout << "  t3 = add i_0, 1" << std::endl;
    std::cout << "  i_0 = move t3" << std::endl;
    std::cout << "  jump while_start_0" << std::endl;
    std::cout << "  while_end_1:" << std::endl;
    std::cout << std::endl;

    {
        // Inner loop: while (j < 2) j = j + 1
        auto j1 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto two = std::make_unique<LiteralExpr>("2", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto innerCondition = std::make_unique<BinaryExpr>(
            std::move(j1),
            "<",
            std::move(two),
            SourceLocation()
        );

        auto j2 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto one1 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto innerAdd = std::make_unique<BinaryExpr>(
            std::move(j2),
            "+",
            std::move(one1),
            SourceLocation()
        );
        auto j3 = std::make_unique<IdentifierExpr>("j", SourceLocation());
        auto innerAssign = std::make_unique<AssignmentExpr>(
            std::move(j3),
            std::move(innerAdd),
            SourceLocation()
        );
        auto innerBody = std::make_unique<ExpressionStmt>(std::move(innerAssign), SourceLocation());

        auto innerWhile = std::make_unique<WhileStmt>(
            std::move(innerCondition),
            std::move(innerBody),
            SourceLocation()
        );

        // Outer loop body: inner loop + i = i + 1
        auto i2 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto one2 = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto outerAdd = std::make_unique<BinaryExpr>(
            std::move(i2),
            "+",
            std::move(one2),
            SourceLocation()
        );
        auto i3 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto outerAssign = std::make_unique<AssignmentExpr>(
            std::move(i3),
            std::move(outerAdd),
            SourceLocation()
        );
        auto outerStmt = std::make_unique<ExpressionStmt>(std::move(outerAssign), SourceLocation());

        std::vector<std::unique_ptr<Statement>> outerStatements;
        outerStatements.push_back(std::move(innerWhile));
        outerStatements.push_back(std::move(outerStmt));
        auto outerBody = std::make_unique<CompoundStmt>(std::move(outerStatements), SourceLocation());

        // Outer loop: while (i < 3)
        auto i1 = std::make_unique<IdentifierExpr>("i", SourceLocation());
        auto three = std::make_unique<LiteralExpr>("3", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto outerCondition = std::make_unique<BinaryExpr>(
            std::move(i1),
            "<",
            std::move(three),
            SourceLocation()
        );

        auto outerWhile = std::make_unique<WhileStmt>(
            std::move(outerCondition),
            std::move(outerBody),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(outerWhile.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== All While Loop Examples Complete ===" << std::endl;

    return 0;
}
