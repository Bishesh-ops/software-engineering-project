#include "ir.h"
#include "ir_optimizer.h"
#include <iostream>
#include <cassert>
#include <memory>

// ============================================================================
// Test Common Subexpression Elimination Optimization
// ============================================================================
// This test suite validates the CSE optimization pass
// User Story: As a compiler, I want to eliminate redundant computations
// so that performance is improved
//
// Acceptance Criteria:
// - t0 = a + b; t1 = a + b; → t0 = a + b; t1 = t0;

void printTestHeader(const std::string& title) {
    std::cout << "\n========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n";
}

void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << "\n";
}

// ============================================================================
// Test 1: Simple Common Subexpression (a + b twice)
// ============================================================================
void testSimpleCSE() {
    printTestHeader("TEST 1: Simple Common Subexpression (a + b)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create variables
    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // t0 = a + b
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = a + b  (duplicate - should be optimized)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(a), IROperand(b)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should have optimized 1 instruction
    bool passed = (optimizations == 1);
    printTestResult("Should eliminate 1 common subexpression", passed);

    // Second instruction should be a MOVE
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool isMove = (instructions[1]->getOpcode() == IROpcode::MOVE);
    printTestResult("Should replace second ADD with MOVE", isMove);

    std::cout << "\n";
}

// ============================================================================
// Test 2: Multiple Common Subexpressions
// ============================================================================
void testMultipleCSE() {
    printTestHeader("TEST 2: Multiple Common Subexpressions");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);

    // t0 = x + y
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(x), IROperand(y)));

    // t1 = x + y  (duplicate)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(x), IROperand(y)));

    // t2 = x + y  (duplicate)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t2, IROperand(x), IROperand(y)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should optimize 2 duplicates
    bool passed = (optimizations == 2);
    printTestResult("Should eliminate 2 common subexpressions", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 3: Different Operations (No CSE)
// ============================================================================
void testDifferentOperations() {
    printTestHeader("TEST 3: Different Operations (No CSE)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // t0 = a + b
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = a - b  (different operation)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t1, IROperand(a), IROperand(b)));

    // t2 = a * b  (different operation)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2, IROperand(a), IROperand(b)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // No optimizations (all different operations)
    bool passed = (optimizations == 0);
    printTestResult("Should not optimize different operations", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 4: Different Operands (No CSE)
// ============================================================================
void testDifferentOperands() {
    printTestHeader("TEST 4: Different Operands (No CSE)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c("c", "int", 0);

    // t0 = a + b
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = a + c  (different second operand)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(a), IROperand(c)));

    // t2 = b + c  (different operands)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t2, IROperand(b), IROperand(c)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // No optimizations (all different operands)
    bool passed = (optimizations == 0);
    printTestResult("Should not optimize different operands", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 5: CSE with Multiplication
// ============================================================================
void testCSEMultiplication() {
    printTestHeader("TEST 5: CSE with Multiplication");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);

    // t0 = x * y
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t0, IROperand(x), IROperand(y)));

    // Some other operation
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(x), IROperand("1", IROperand::OperandType::CONSTANT)));

    // t2 = x * y  (duplicate multiplication)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2, IROperand(x), IROperand(y)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should eliminate the duplicate multiplication
    bool passed = (optimizations == 1);
    printTestResult("Should eliminate duplicate multiplication", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 6: CSE with Comparison Operations
// ============================================================================
void testCSEComparison() {
    printTestHeader("TEST 6: CSE with Comparison Operations");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // t0 = a < b
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::LT, &t0, IROperand(a), IROperand(b)));

    // t1 = a < b  (duplicate comparison)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::LT, &t1, IROperand(a), IROperand(b)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 1);
    printTestResult("Should eliminate duplicate comparison", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 7: CSE with Constants
// ============================================================================
void testCSEWithConstants() {
    printTestHeader("TEST 7: CSE with Constants");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);

    // t0 = x + 5
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(x),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    // t1 = x + 5  (duplicate)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(x),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (optimizations == 1);
    printTestResult("Should eliminate expression with constants", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 8: No CSE Across Basic Blocks (Conservative)
// ============================================================================
void testNoCSEAcrossBlocks() {
    printTestHeader("TEST 8: No CSE Across Basic Blocks");

    IRFunction func("test", "int");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);

    // Block 1
    auto block1 = std::make_unique<IRBasicBlock>("block1");
    SSAValue t0("t", "int", 0);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(x), IROperand(y)));
    func.addBasicBlock(std::move(block1));

    // Block 2 (separate block)
    auto block2 = std::make_unique<IRBasicBlock>("block2");
    SSAValue t1("t", "int", 1);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(x), IROperand(y)));
    func.addBasicBlock(std::move(block2));

    std::cout << "Before CSE:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.toString() << "\n";

    // Our simple CSE doesn't optimize across blocks (conservative)
    bool passed = (optimizations == 0);
    printTestResult("Should not optimize across blocks (conservative)", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 9: Integration with Constant Folding
// ============================================================================
void testIntegrationWithConstantFolding() {
    printTestHeader("TEST 9: Integration with Constant Folding");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // t0 = 2 + 3  (will be folded)
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("2", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    // t1 = 2 + 3  (will be folded, then CSE'd)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1,
        IROperand("2", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Run full pipeline
    IROptimizer optimizer;
    optimizer.optimize(&func);

    std::cout << "After full optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Check results
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();

    // Both should be MOVE instructions after constant folding
    bool bothFolded = (instructions[0]->getOpcode() == IROpcode::MOVE &&
                       instructions[1]->getOpcode() == IROpcode::MOVE);
    printTestResult("Both should be constant folded to MOVE", bothFolded);

    std::cout << "\n";
}

// ============================================================================
// Test 10: Complex Expression Chain
// ============================================================================
void testComplexChain() {
    printTestHeader("TEST 10: Complex Expression Chain");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c("c", "int", 0);

    // t0 = a + b
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = c * 2
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1, IROperand(c),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    // t2 = a + b  (duplicate of t0)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t2, IROperand(a), IROperand(b)));

    // t3 = c * 2  (duplicate of t1)
    SSAValue t3("t", "int", 3);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t3, IROperand(c),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    std::cout << "Before CSE:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should eliminate both duplicates
    bool passed = (optimizations == 2);
    printTestResult("Should eliminate 2 duplicates", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 11: Optimization Statistics
// ============================================================================
void testOptimizationStatistics() {
    printTestHeader("TEST 11: Optimization Statistics");

    IROptimizer optimizer;

    // Function 1: 1 CSE opportunity
    IRFunction func1("func1", "int");
    auto block1 = std::make_unique<IRBasicBlock>("entry");
    SSAValue x1("x", "int", 0);
    SSAValue y1("y", "int", 0);
    SSAValue t0("t", "int", 0);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(x1), IROperand(y1)));
    SSAValue t1("t", "int", 1);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(x1), IROperand(y1)));
    func1.addBasicBlock(std::move(block1));
    optimizer.commonSubexpressionEliminationPass(&func1);

    // Function 2: 2 CSE opportunities
    IRFunction func2("func2", "int");
    auto block2 = std::make_unique<IRBasicBlock>("entry");
    SSAValue x2("x", "int", 0);
    SSAValue y2("y", "int", 0);
    SSAValue t2("t", "int", 2);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2, IROperand(x2), IROperand(y2)));
    SSAValue t3("t", "int", 3);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t3, IROperand(x2), IROperand(y2)));
    SSAValue t4("t", "int", 4);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t4, IROperand(x2), IROperand(y2)));
    func2.addBasicBlock(std::move(block2));
    optimizer.commonSubexpressionEliminationPass(&func2);

    std::cout << "\n";
    optimizer.printOptimizationReport();

    // Total should be 3 (1 + 2)
    bool passed = (optimizer.getCSECount() == 3);
    printTestResult("Should track 3 total CSE optimizations", passed);

    std::cout << "\n";
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "COMMON SUBEXPRESSION ELIMINATION TEST SUITE\n";
    std::cout << "========================================\n";
    std::cout << "User Story: As a compiler, I want to\n";
    std::cout << "eliminate redundant computations so that\n";
    std::cout << "performance is improved.\n";
    std::cout << "\n";
    std::cout << "Acceptance Criteria:\n";
    std::cout << "- t0 = a + b; t1 = a + b;\n";
    std::cout << "  → t0 = a + b; t1 = t0;\n";

    // Run all tests
    testSimpleCSE();
    testMultipleCSE();
    testDifferentOperations();
    testDifferentOperands();
    testCSEMultiplication();
    testCSEComparison();
    testCSEWithConstants();
    testNoCSEAcrossBlocks();
    testIntegrationWithConstantFolding();
    testComplexChain();
    testOptimizationStatistics();

    std::cout << "========================================\n";
    std::cout << "All CSE Tests Complete!\n";
    std::cout << "========================================\n";

    return 0;
}
