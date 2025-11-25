#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// If Statement Lowering Example
// ============================================================================
// Demonstrates how if statements are lowered to IR with conditional jumps
//
// Acceptance Criteria:
// 1. if (cond) then_stmt →
//      eval cond → tc
//      JUMP_IF_FALSE tc, L_end
//      then_stmt
//      L_end:
//
// 2. if (cond) then_stmt else else_stmt →
//      eval cond → tc
//      JUMP_IF_FALSE tc, L_else
//      then_stmt
//      JUMP L_end
//      L_else:
//      else_stmt
//      L_end:

int main() {
    std::cout << "=== If Statement Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple If Statement (no else)
    // ========================================================================
    std::cout << "Example 1: Simple If Statement (no else)" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  if (x < 10)" << std::endl;
    std::cout << "    y = 1;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = lt x_0, 10" << std::endl;
    std::cout << "  jump_if_false t0, if_end_0" << std::endl;
    std::cout << "  t1 = move 1" << std::endl;
    std::cout << "  y_0 = move t1" << std::endl;
    std::cout << "  if_end_0:" << std::endl;
    std::cout << std::endl;

    {
        // Condition: x < 10
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto ten = std::make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(x),
            "<",
            std::move(ten),
            SourceLocation()
        );

        // Then body: y = 1
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto y = std::make_unique<IdentifierExpr>("y", SourceLocation());
        auto assignment = std::make_unique<AssignmentExpr>(
            std::move(y),
            std::move(one),
            SourceLocation()
        );
        auto thenStmt = std::make_unique<ExpressionStmt>(std::move(assignment), SourceLocation());

        // If statement
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition),
            std::move(thenStmt),
            nullptr,  // no else
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(ifStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 2: If-Else Statement
    // ========================================================================
    std::cout << "Example 2: If-Else Statement" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  if (a > b)" << std::endl;
    std::cout << "    max = a;" << std::endl;
    std::cout << "  else" << std::endl;
    std::cout << "    max = b;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = gt a_0, b_0" << std::endl;
    std::cout << "  jump_if_false t0, if_else_0" << std::endl;
    std::cout << "  max_0 = move a_0" << std::endl;
    std::cout << "  jump if_end_1" << std::endl;
    std::cout << "  if_else_0:" << std::endl;
    std::cout << "  max_0 = move b_0" << std::endl;
    std::cout << "  if_end_1:" << std::endl;
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

        // Then: max = a
        auto a2 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto max1 = std::make_unique<IdentifierExpr>("max", SourceLocation());
        auto thenAssign = std::make_unique<AssignmentExpr>(
            std::move(max1),
            std::move(a2),
            SourceLocation()
        );
        auto thenStmt = std::make_unique<ExpressionStmt>(std::move(thenAssign), SourceLocation());

        // Else: max = b
        auto b2 = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto max2 = std::make_unique<IdentifierExpr>("max", SourceLocation());
        auto elseAssign = std::make_unique<AssignmentExpr>(
            std::move(max2),
            std::move(b2),
            SourceLocation()
        );
        auto elseStmt = std::make_unique<ExpressionStmt>(std::move(elseAssign), SourceLocation());

        // If-else statement
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition),
            std::move(thenStmt),
            std::move(elseStmt),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(ifStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 3: If with Multiple Statements (Compound)
    // ========================================================================
    std::cout << "Example 3: If with Multiple Statements" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  if (x == 0) {" << std::endl;
    std::cout << "    y = 10;" << std::endl;
    std::cout << "    z = 20;" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = eq x_0, 0" << std::endl;
    std::cout << "  jump_if_false t0, if_end_0" << std::endl;
    std::cout << "  t1 = move 10" << std::endl;
    std::cout << "  y_0 = move t1" << std::endl;
    std::cout << "  t2 = move 20" << std::endl;
    std::cout << "  z_0 = move t2" << std::endl;
    std::cout << "  if_end_0:" << std::endl;
    std::cout << std::endl;

    {
        // Condition: x == 0
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto zero = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(x),
            "==",
            std::move(zero),
            SourceLocation()
        );

        // Statement 1: y = 10
        auto ten = std::make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto y = std::make_unique<IdentifierExpr>("y", SourceLocation());
        auto assign1 = std::make_unique<AssignmentExpr>(
            std::move(y),
            std::move(ten),
            SourceLocation()
        );
        auto stmt1 = std::make_unique<ExpressionStmt>(std::move(assign1), SourceLocation());

        // Statement 2: z = 20
        auto twenty = std::make_unique<LiteralExpr>("20", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto z = std::make_unique<IdentifierExpr>("z", SourceLocation());
        auto assign2 = std::make_unique<AssignmentExpr>(
            std::move(z),
            std::move(twenty),
            SourceLocation()
        );
        auto stmt2 = std::make_unique<ExpressionStmt>(std::move(assign2), SourceLocation());

        // Compound statement
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(stmt1));
        statements.push_back(std::move(stmt2));
        auto compoundStmt = std::make_unique<CompoundStmt>(std::move(statements), SourceLocation());

        // If statement
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition),
            std::move(compoundStmt),
            nullptr,
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(ifStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 4: If-Else with Complex Expression
    // ========================================================================
    std::cout << "Example 4: If-Else with Complex Expression" << std::endl;
    std::cout << "AST:" << std::endl;
    std::cout << "  if (x + y > 100)" << std::endl;
    std::cout << "    result = 1;" << std::endl;
    std::cout << "  else" << std::endl;
    std::cout << "    result = 0;" << std::endl;
    std::cout << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add x_0, y_0" << std::endl;
    std::cout << "  t1 = move 100" << std::endl;
    std::cout << "  t2 = gt t0, t1" << std::endl;
    std::cout << "  jump_if_false t2, if_else_0" << std::endl;
    std::cout << "  t3 = move 1" << std::endl;
    std::cout << "  result_0 = move t3" << std::endl;
    std::cout << "  jump if_end_1" << std::endl;
    std::cout << "  if_else_0:" << std::endl;
    std::cout << "  t4 = move 0" << std::endl;
    std::cout << "  result_0 = move t4" << std::endl;
    std::cout << "  if_end_1:" << std::endl;
    std::cout << std::endl;

    {
        // Condition: (x + y) > 100
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto y = std::make_unique<IdentifierExpr>("y", SourceLocation());
        auto sum = std::make_unique<BinaryExpr>(
            std::move(x),
            "+",
            std::move(y),
            SourceLocation()
        );
        auto hundred = std::make_unique<LiteralExpr>("100", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto condition = std::make_unique<BinaryExpr>(
            std::move(sum),
            ">",
            std::move(hundred),
            SourceLocation()
        );

        // Then: result = 1
        auto one = std::make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto result1 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto thenAssign = std::make_unique<AssignmentExpr>(
            std::move(result1),
            std::move(one),
            SourceLocation()
        );
        auto thenStmt = std::make_unique<ExpressionStmt>(std::move(thenAssign), SourceLocation());

        // Else: result = 0
        auto zero = std::make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, SourceLocation());
        auto result2 = std::make_unique<IdentifierExpr>("result", SourceLocation());
        auto elseAssign = std::make_unique<AssignmentExpr>(
            std::move(result2),
            std::move(zero),
            SourceLocation()
        );
        auto elseStmt = std::make_unique<ExpressionStmt>(std::move(elseAssign), SourceLocation());

        // If-else statement
        auto ifStmt = std::make_unique<IfStmt>(
            std::move(condition),
            std::move(thenStmt),
            std::move(elseStmt),
            SourceLocation()
        );

        auto irInstructions = codegen.generateStatementIR(ifStmt.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== All If Statement Examples Complete ===" << std::endl;

    return 0;
}
