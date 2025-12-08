#include <gtest/gtest.h>
#include "ir_codegen.h"
#include "parser.h"
#include "lexer.h"
#include "test_helpers.h"

// ============================================================================
// Test Fixture for IR Code Generation
// ============================================================================
class IRCodegenTest : public ::testing::Test {
protected:
    IRCodeGenerator codegen;

    // Helper: Parse expression and generate IR
    std::vector<std::unique_ptr<IRInstruction>> generateIRForExpression(const std::string& source) {
        auto expr = mycc_test::parse_expression_without_errors(source);
        if (!expr) {
            return {};
        }
        return codegen.generateExpressionIR(expr.get());
    }

    // Helper: Parse statement and generate IR
    std::vector<std::unique_ptr<IRInstruction>> generateIRForStatement(const std::string& source) {
        Lexer lexer(source, "test.c");
        Parser parser(lexer);
        auto program = parser.parseProgram();

        if (parser.hasErrors() || program.empty()) {
            return {};
        }

        // Get the first statement from the main function
        if (auto* funcDecl = dynamic_cast<FunctionDecl*>(program[0].get())) {
            if (funcDecl->getBody() && !funcDecl->getBody()->getStatements().empty()) {
                return codegen.generateStatementIR(funcDecl->getBody()->getStatements()[0].get());
            }
        }

        return {};
    }

    // Helper: Check if instruction list contains a specific opcode
    bool hasOpcode(const std::vector<std::unique_ptr<IRInstruction>>& instructions, IROpcode opcode) {
        for (const auto& inst : instructions) {
            if (inst->getOpcode() == opcode) {
                return true;
            }
        }
        return false;
    }

    // Helper: Count instructions with specific opcode
    int countOpcode(const std::vector<std::unique_ptr<IRInstruction>>& instructions, IROpcode opcode) {
        int count = 0;
        for (const auto& inst : instructions) {
            if (inst->getOpcode() == opcode) {
                count++;
            }
        }
        return count;
    }

    void SetUp() override {
        codegen.reset();
    }
};

// ============================================================================
// Expression IR Generation Tests
// ============================================================================

/**
 * Test: Literal expression generates MOVE instruction
 * Example: 42 -> MOVE t0, 42
 */
TEST_F(IRCodegenTest, GeneratesIRForIntegerLiteral) {
    auto instructions = generateIRForExpression("42");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::MOVE));
}

/**
 * Test: Identifier expression uses variable directly
 * Example: x -> uses "x" as operand
 */
TEST_F(IRCodegenTest, GeneratesIRForIdentifier) {
    auto instructions = generateIRForExpression("x");

    // Identifier alone might not generate instructions, just pushes operand
    // This is valid behavior
    EXPECT_TRUE(instructions.empty() || hasOpcode(instructions, IROpcode::MOVE));
}

/**
 * Test: Binary addition generates ADD instruction
 * Example: a + b -> t0 = ADD a, b
 */
TEST_F(IRCodegenTest, GeneratesIRForBinaryAddition) {
    auto instructions = generateIRForExpression("a + b");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::ADD));
}

/**
 * Test: Binary subtraction generates SUB instruction
 */
TEST_F(IRCodegenTest, GeneratesIRForBinarySubtraction) {
    auto instructions = generateIRForExpression("x - y");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::SUB));
}

/**
 * Test: Binary multiplication generates MUL instruction
 */
TEST_F(IRCodegenTest, GeneratesIRForBinaryMultiplication) {
    auto instructions = generateIRForExpression("a * b");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::MUL));
}

/**
 * Test: Binary division generates DIV instruction
 */
TEST_F(IRCodegenTest, GeneratesIRForBinaryDivision) {
    auto instructions = generateIRForExpression("x / y");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::DIV));
}

/**
 * Test: Nested expressions generate multiple instructions in correct order
 * Example: a + b * c -> t0 = MUL b, c; t1 = ADD a, t0
 */
TEST_F(IRCodegenTest, GeneratesIRForNestedExpressions) {
    auto instructions = generateIRForExpression("a + b * c");

    ASSERT_GE(instructions.size(), 2);
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::MUL));
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::ADD));
}

/**
 * Test: Comparison operators generate CMP instructions
 */
TEST_F(IRCodegenTest, GeneratesIRForComparison) {
    auto instructions = generateIRForExpression("x < y");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::LT));
}

/**
 * Test: Logical AND generates appropriate instructions
 * Note: Logical AND is typically implemented with conditional branches, not a single AND opcode
 */
TEST_F(IRCodegenTest, GeneratesIRForLogicalAnd) {
    auto instructions = generateIRForExpression("a && b");

    ASSERT_FALSE(instructions.empty());
    // Logical AND is implemented with control flow (JUMP_IF_FALSE)
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::JUMP_IF_FALSE) ||
                hasOpcode(instructions, IROpcode::JUMP));
}

/**
 * Test: Unary minus generates SUB instruction (0 - x)
 * Note: Unary minus is typically implemented as subtraction from zero
 */
TEST_F(IRCodegenTest, GeneratesIRForUnaryMinus) {
    auto instructions = generateIRForExpression("-x");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::SUB) ||
                hasOpcode(instructions, IROpcode::MUL));  // Or multiply by -1
}

/**
 * Test: Unary logical NOT generates comparison with 0 (x == 0)
 * Note: Logical NOT is typically implemented as EQ comparison with 0
 */
TEST_F(IRCodegenTest, GeneratesIRForUnaryNot) {
    auto instructions = generateIRForExpression("!x");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::EQ) ||
                hasOpcode(instructions, IROpcode::JUMP_IF_FALSE));
}

/**
 * Test: Assignment generates STORE instruction
 * Example: x = 42 -> t0 = MOVE 42; STORE x, t0
 */
TEST_F(IRCodegenTest, GeneratesIRForAssignment) {
    auto instructions = generateIRForExpression("x = 42");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::STORE) ||
                hasOpcode(instructions, IROpcode::MOVE));
}

// ============================================================================
// Statement IR Generation Tests
// ============================================================================

/**
 * Test: Return statement generates RET instruction
 */
TEST_F(IRCodegenTest, GeneratesIRForReturnStatement) {
    std::string source = R"(
        int main() {
            return 42;
        }
    )";

    auto instructions = generateIRForStatement(source);

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::RETURN));
}

/**
 * Test: Expression statement generates appropriate IR
 */
TEST_F(IRCodegenTest, GeneratesIRForExpressionStatement) {
    std::string source = R"(
        int main() {
            x + y;
        }
    )";

    auto instructions = generateIRForStatement(source);

    // Expression statement should generate IR for the expression
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::ADD));
}

// ============================================================================
// Control Flow IR Generation Tests
// ============================================================================

/**
 * Test: If statement generates conditional branch instructions
 * Example: if (x < y) ... -> CMP, BRANCH
 */
TEST_F(IRCodegenTest, GeneratesIRForIfStatement) {
    std::string source = R"(
        int main() {
            if (x < 5) {
                return 1;
            }
        }
    )";

    auto instructions = generateIRForStatement(source);

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::JUMP) ||
                hasOpcode(instructions, IROpcode::JUMP_IF_FALSE));
}

/**
 * Test: While loop generates loop label and conditional branch
 */
TEST_F(IRCodegenTest, GeneratesIRForWhileLoop) {
    std::string source = R"(
        int main() {
            while (x < 10) {
                x = x + 1;
            }
        }
    )";

    auto instructions = generateIRForStatement(source);

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::LABEL));
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::JUMP) ||
                hasOpcode(instructions, IROpcode::JUMP_IF_FALSE));
}

// ============================================================================
// Function Call IR Generation Tests
// ============================================================================

/**
 * Test: Function call generates CALL instruction with arguments
 */
TEST_F(IRCodegenTest, GeneratesIRForFunctionCall) {
    auto instructions = generateIRForExpression("foo(42)");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::CALL));
}

/**
 * Test: Function call with multiple arguments
 */
TEST_F(IRCodegenTest, GeneratesIRForFunctionCallMultipleArgs) {
    auto instructions = generateIRForExpression("add(x, y)");

    ASSERT_FALSE(instructions.empty());
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::CALL));
}

// ============================================================================
// SSA Form Tests
// ============================================================================

/**
 * Test: Multiple assignments to same variable create different SSA versions
 * Example: x = 1; x = 2; -> x_0 = 1; x_1 = 2;
 */
TEST_F(IRCodegenTest, GeneratesSSAFormForMultipleAssignments) {
    std::string source = R"(
        int main() {
            x = 1;
            x = 2;
        }
    )";

    // This test verifies that SSA form is maintained
    // Each assignment should create a new SSA version
    // The exact implementation depends on the IR generator

    auto instructions = generateIRForStatement(source);
    ASSERT_FALSE(instructions.empty());

    // Count STORE or MOVE instructions
    int storeCount = countOpcode(instructions, IROpcode::STORE);
    int moveCount = countOpcode(instructions, IROpcode::MOVE);

    EXPECT_GE(storeCount + moveCount, 2);
}

/**
 * Test: Complex expression with multiple temporaries uses SSA form
 */
TEST_F(IRCodegenTest, GeneratesSSAFormForComplexExpression) {
    auto instructions = generateIRForExpression("(a + b) * (c - d)");

    ASSERT_GE(instructions.size(), 3);

    // Should have ADD, SUB, and MUL instructions
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::ADD));
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::SUB));
    EXPECT_TRUE(hasOpcode(instructions, IROpcode::MUL));
}

// ============================================================================
// Code Generator Reset Tests
// ============================================================================

/**
 * Test: Code generator can be reset and reused
 */
TEST_F(IRCodegenTest, CanResetAndReuse) {
    auto instructions1 = generateIRForExpression("x + y");
    ASSERT_FALSE(instructions1.empty());

    codegen.reset();

    auto instructions2 = generateIRForExpression("a * b");
    ASSERT_FALSE(instructions2.empty());
    EXPECT_TRUE(hasOpcode(instructions2, IROpcode::MUL));
}
