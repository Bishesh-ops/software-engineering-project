#include "ir_codegen.h"
#include "ast.h"
#include <iostream>
#include <memory>

// ============================================================================
// Expression Lowering Example
// ============================================================================
// Demonstrates how AST expressions are lowered to IR instructions
//
// This example shows the acceptance criteria:
// 1. Literals → MOVE t0, 42
// 2. Identifiers → use name directly
// 3. Binary ops → t0 = left OP right
// 4. Nested expressions use multiple instructions
//
// Example: a + b * c
// Becomes:
//   t0 = mul b, c
//   t1 = add a, t0

int main() {
    std::cout << "=== Expression Lowering to IR Examples ===" << std::endl;
    std::cout << std::endl;

    IRCodeGenerator codegen;

    // ========================================================================
    // Example 1: Simple Literal
    // ========================================================================
    std::cout << "Example 1: Literal Expression" << std::endl;
    std::cout << "AST: 42" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 42" << std::endl;
    std::cout << std::endl;

    {
        auto literal = std::make_unique<LiteralExpr>(
            "42",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(literal.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 2: Identifier
    // ========================================================================
    std::cout << "Example 2: Identifier Expression" << std::endl;
    std::cout << "AST: x" << std::endl;
    std::cout << "Expected: Identifier 'x' used directly (no instruction)" << std::endl;
    std::cout << std::endl;

    {
        auto identifier = std::make_unique<IdentifierExpr>("x", SourceLocation());

        auto irInstructions = codegen.generateExpressionIR(identifier.get());

        std::cout << "Generated IR:" << std::endl;
        if (irInstructions.empty()) {
            std::cout << "  (no instructions - identifier used directly)" << std::endl;
        } else {
            for (const auto& inst : irInstructions) {
                std::cout << "  " << inst->toString() << std::endl;
            }
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 3: Simple Binary Operation
    // ========================================================================
    std::cout << "Example 3: Simple Binary Operation" << std::endl;
    std::cout << "AST: a + b" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add a_0, b_0" << std::endl;
    std::cout << std::endl;

    {
        auto left = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto right = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto binaryExpr = std::make_unique<BinaryExpr>(
            std::move(left),
            "+",
            std::move(right),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(binaryExpr.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 4: Nested Expression (a + b * c)
    // ========================================================================
    std::cout << "Example 4: Nested Expression (MAIN ACCEPTANCE CRITERIA)" << std::endl;
    std::cout << "AST: a + (b * c)" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = mul b_0, c_0" << std::endl;
    std::cout << "  t1 = add a_0, t0" << std::endl;
    std::cout << std::endl;

    {
        // Create: b * c
        auto b = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto c = std::make_unique<IdentifierExpr>("c", SourceLocation());
        auto multiply = std::make_unique<BinaryExpr>(
            std::move(b),
            "*",
            std::move(c),
            SourceLocation()
        );

        // Create: a + (b * c)
        auto a = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto addition = std::make_unique<BinaryExpr>(
            std::move(a),
            "+",
            std::move(multiply),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(addition.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 5: More Complex Nested Expression
    // ========================================================================
    std::cout << "Example 5: More Complex Nested Expression" << std::endl;
    std::cout << "AST: (a + b) * (c - d)" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = add a_0, b_0" << std::endl;
    std::cout << "  t1 = sub c_0, d_0" << std::endl;
    std::cout << "  t2 = mul t0, t1" << std::endl;
    std::cout << std::endl;

    {
        // Create: a + b
        auto a1 = std::make_unique<IdentifierExpr>("a", SourceLocation());
        auto b1 = std::make_unique<IdentifierExpr>("b", SourceLocation());
        auto addition = std::make_unique<BinaryExpr>(
            std::move(a1),
            "+",
            std::move(b1),
            SourceLocation()
        );

        // Create: c - d
        auto c1 = std::make_unique<IdentifierExpr>("c", SourceLocation());
        auto d1 = std::make_unique<IdentifierExpr>("d", SourceLocation());
        auto subtraction = std::make_unique<BinaryExpr>(
            std::move(c1),
            "-",
            std::move(d1),
            SourceLocation()
        );

        // Create: (a + b) * (c - d)
        auto multiply = std::make_unique<BinaryExpr>(
            std::move(addition),
            "*",
            std::move(subtraction),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(multiply.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 6: Comparison Operations
    // ========================================================================
    std::cout << "Example 6: Comparison Operation" << std::endl;
    std::cout << "AST: x < y" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = lt x_0, y_0" << std::endl;
    std::cout << std::endl;

    {
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto y = std::make_unique<IdentifierExpr>("y", SourceLocation());
        auto comparison = std::make_unique<BinaryExpr>(
            std::move(x),
            "<",
            std::move(y),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(comparison.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    // ========================================================================
    // Example 7: Mixed Literals and Variables
    // ========================================================================
    std::cout << "Example 7: Mixed Literals and Variables" << std::endl;
    std::cout << "AST: x + 10" << std::endl;
    std::cout << "Expected IR:" << std::endl;
    std::cout << "  t0 = move 10" << std::endl;
    std::cout << "  t1 = add x_0, t0" << std::endl;
    std::cout << std::endl;

    {
        auto x = std::make_unique<IdentifierExpr>("x", SourceLocation());
        auto ten = std::make_unique<LiteralExpr>(
            "10",
            LiteralExpr::LiteralType::INTEGER,
            SourceLocation()
        );
        auto addition = std::make_unique<BinaryExpr>(
            std::move(x),
            "+",
            std::move(ten),
            SourceLocation()
        );

        auto irInstructions = codegen.generateExpressionIR(addition.get());

        std::cout << "Generated IR:" << std::endl;
        for (const auto& inst : irInstructions) {
            std::cout << "  " << inst->toString() << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== All Examples Complete ===" << std::endl;

    return 0;
}
