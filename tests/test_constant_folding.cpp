#include "ir.h"
#include "ir_optimizer.h"
#include <iostream>
#include <cassert>
#include <memory>

// ============================================================================
// Test Constant Folding Optimization
// ============================================================================
// This test suite validates the constant folding optimization pass
// User Story: As a compiler, I want to evaluate constant expressions at
// compile time so that runtime computation is reduced

void printTestHeader(const std::string& title) {
    std::cout << "\n========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n";
}

void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << "\n";
}

// ============================================================================
// Test 1: Simple Addition (2 + 3 → 5)
// ============================================================================
void testSimpleAddition() {
    printTestHeader("TEST 1: Simple Addition (2 + 3 → 5)");

    // Create a function with a single basic block
    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = add 2, 3
    SSAValue result("t", "int", 0);
    IROperand op1("2", IROperand::OperandType::CONSTANT);
    IROperand op2("3", IROperand::OperandType::CONSTANT);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, op1, op2));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply constant folding
    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Verify: Should have optimized 1 instruction
    bool passed = (optimizations == 1);
    printTestResult("Should optimize 2 + 3 to 5", passed);

    // Verify the instruction was replaced with MOVE
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool isMoveInst = (instructions[0]->getOpcode() == IROpcode::MOVE);
    printTestResult("Should replace ADD with MOVE", isMoveInst);

    std::cout << "\n";
}

// ============================================================================
// Test 2: Multiplication by Zero (10 * 0 → 0)
// ============================================================================
void testMultiplicationByZero() {
    printTestHeader("TEST 2: Multiplication by Zero (10 * 0 → 0)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = mul 10, 0
    SSAValue result("t", "int", 0);
    IROperand op1("10", IROperand::OperandType::CONSTANT);
    IROperand op2("0", IROperand::OperandType::CONSTANT);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &result, op1, op2));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply constant folding
    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Verify optimization
    bool passed = (optimizations == 1);
    printTestResult("Should optimize 10 * 0 to 0", passed);

    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool isMoveInst = (instructions[0]->getOpcode() == IROpcode::MOVE);
    printTestResult("Should replace MUL with MOVE", isMoveInst);

    std::cout << "\n";
}

// ============================================================================
// Test 3: Subtraction (15 - 7 → 8)
// ============================================================================
void testSubtraction() {
    printTestHeader("TEST 3: Subtraction (15 - 7 → 8)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = sub 15, 7
    SSAValue result("t", "int", 0);
    IROperand op1("15", IROperand::OperandType::CONSTANT);
    IROperand op2("7", IROperand::OperandType::CONSTANT);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &result, op1, op2));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 1);
    printTestResult("Should optimize 15 - 7 to 8", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 4: Division (20 / 4 → 5)
// ============================================================================
void testDivision() {
    printTestHeader("TEST 4: Division (20 / 4 → 5)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = div 20, 4
    SSAValue result("t", "int", 0);
    IROperand op1("20", IROperand::OperandType::CONSTANT);
    IROperand op2("4", IROperand::OperandType::CONSTANT);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &result, op1, op2));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 1);
    printTestResult("Should optimize 20 / 4 to 5", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 5: Modulo (17 % 5 → 2)
// ============================================================================
void testModulo() {
    printTestHeader("TEST 5: Modulo (17 % 5 → 2)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = mod 17, 5
    SSAValue result("t", "int", 0);
    IROperand op1("17", IROperand::OperandType::CONSTANT);
    IROperand op2("5", IROperand::OperandType::CONSTANT);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MOD, &result, op1, op2));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 1);
    printTestResult("Should optimize 17 % 5 to 2", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 6: Multiple Constant Operations
// ============================================================================
void testMultipleOperations() {
    printTestHeader("TEST 6: Multiple Constant Operations");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create multiple constant operations:
    // t0 = add 1, 2     → t0 = move 3
    // t1 = mul 5, 6     → t1 = move 30
    // t2 = sub 10, 3    → t2 = move 7

    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("6", IROperand::OperandType::CONSTANT)));

    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // All 3 operations should be optimized
    bool passed = (optimizations == 3);
    printTestResult("Should optimize all 3 constant operations", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 7: Mixed Operations (Constants and Variables)
// ============================================================================
void testMixedOperations() {
    printTestHeader("TEST 7: Mixed Operations (Constants and Variables)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create mixed operations:
    // t0 = add 2, 3        → Should optimize (both constants)
    // t1 = add x, 5        → Should NOT optimize (x is variable)
    // t2 = mul 4, 7        → Should optimize (both constants)

    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("2", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    SSAValue x("x", "int", 0);
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1,
        IROperand(x),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2,
        IROperand("4", IROperand::OperandType::CONSTANT),
        IROperand("7", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Only 2 operations should be optimized (the ones with all constants)
    bool passed = (optimizations == 2);
    printTestResult("Should optimize only constant-only operations", passed);

    // Verify middle instruction (with variable) is still ADD
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool middleUnchanged = (instructions[1]->getOpcode() == IROpcode::ADD);
    printTestResult("Variable operation should remain unchanged", middleUnchanged);

    std::cout << "\n";
}

// ============================================================================
// Test 8: Division by Zero (Should NOT optimize)
// ============================================================================
void testDivisionByZero() {
    printTestHeader("TEST 8: Division by Zero (Should NOT optimize)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = div 10, 0 (should NOT optimize)
    SSAValue result("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &result,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("0", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should NOT optimize division by zero
    bool passed = (optimizations == 0);
    printTestResult("Should NOT optimize division by zero", passed);

    // Instruction should remain as DIV
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool stillDiv = (instructions[0]->getOpcode() == IROpcode::DIV);
    printTestResult("Instruction should remain as DIV", stillDiv);

    std::cout << "\n";
}

// ============================================================================
// Test 9: Modulo by Zero (Should NOT optimize)
// ============================================================================
void testModuloByZero() {
    printTestHeader("TEST 9: Modulo by Zero (Should NOT optimize)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create instruction: t0 = mod 10, 0 (should NOT optimize)
    SSAValue result("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MOD, &result,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("0", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 0);
    printTestResult("Should NOT optimize modulo by zero", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 10: Negative Numbers
// ============================================================================
void testNegativeNumbers() {
    printTestHeader("TEST 10: Negative Numbers");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create operations with negative numbers:
    // t0 = add -5, 10    → t0 = move 5
    // t1 = mul -3, -4    → t1 = move 12
    // t2 = sub -2, -7    → t2 = move 5

    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("-5", IROperand::OperandType::CONSTANT),
        IROperand("10", IROperand::OperandType::CONSTANT)));

    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("-3", IROperand::OperandType::CONSTANT),
        IROperand("-4", IROperand::OperandType::CONSTANT)));

    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("-2", IROperand::OperandType::CONSTANT),
        IROperand("-7", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 3);
    printTestResult("Should handle negative numbers correctly", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 11: Multiple Basic Blocks
// ============================================================================
void testMultipleBasicBlocks() {
    printTestHeader("TEST 11: Multiple Basic Blocks");

    IRFunction func("test", "int");

    // Block 1: entry
    auto block1 = std::make_unique<IRBasicBlock>("entry");
    SSAValue t0("t", "int", 0);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));
    func.addBasicBlock(std::move(block1));

    // Block 2: then_block
    auto block2 = std::make_unique<IRBasicBlock>("then_block");
    SSAValue t1("t", "int", 1);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("3", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));
    func.addBasicBlock(std::move(block2));

    // Block 3: else_block
    auto block3 = std::make_unique<IRBasicBlock>("else_block");
    SSAValue t2("t", "int", 2);
    block3->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));
    func.addBasicBlock(std::move(block3));

    std::cout << "Before optimization:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.toString() << "\n";

    // All 3 blocks should be optimized (1 instruction each)
    bool passed = (optimizations == 3);
    printTestResult("Should optimize across all basic blocks", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 12: Optimization Statistics
// ============================================================================
void testOptimizationStatistics() {
    printTestHeader("TEST 12: Optimization Statistics");

    IROptimizer optimizer;

    // Create first function with 2 optimizable operations
    IRFunction func1("func1", "int");
    auto block1 = std::make_unique<IRBasicBlock>("entry");
    SSAValue t0("t", "int", 0);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));
    SSAValue t1("t", "int", 1);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("3", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));
    func1.addBasicBlock(std::move(block1));

    optimizer.constantFoldingPass(&func1);

    // Create second function with 1 optimizable operation
    IRFunction func2("func2", "int");
    auto block2 = std::make_unique<IRBasicBlock>("entry");
    SSAValue t2("t", "int", 2);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));
    func2.addBasicBlock(std::move(block2));

    optimizer.constantFoldingPass(&func2);

    std::cout << "\n";
    optimizer.printOptimizationReport();

    // Should have 3 total optimizations
    bool passed = (optimizer.getConstantFoldingCount() == 3);
    printTestResult("Should track total optimizations correctly", passed);

    std::cout << "\n";
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "CONSTANT FOLDING OPTIMIZATION TEST SUITE\n";
    std::cout << "========================================\n";
    std::cout << "User Story: As a compiler, I want to evaluate\n";
    std::cout << "constant expressions at compile time so that\n";
    std::cout << "runtime computation is reduced.\n";

    // Run all tests
    testSimpleAddition();
    testMultiplicationByZero();
    testSubtraction();
    testDivision();
    testModulo();
    testMultipleOperations();
    testMixedOperations();
    testDivisionByZero();
    testModuloByZero();
    testNegativeNumbers();
    testMultipleBasicBlocks();
    testOptimizationStatistics();

    std::cout << "========================================\n";
    std::cout << "All Constant Folding Tests Complete!\n";
    std::cout << "========================================\n";

    return 0;
}
