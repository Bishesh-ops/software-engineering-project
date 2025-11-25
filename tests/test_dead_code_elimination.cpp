#include "ir.h"
#include "ir_optimizer.h"
#include <iostream>
#include <cassert>
#include <memory>

// ============================================================================
// Test Dead Code Elimination Optimization
// ============================================================================
// This test suite validates the dead code elimination optimization pass
// User Story: As a compiler, I want to remove unreachable code so that
// output is cleaner
//
// Acceptance Criteria:
// - Code after unconditional jump removed
// - Code after return removed

void printTestHeader(const std::string& title) {
    std::cout << "\n========================================\n";
    std::cout << title << "\n";
    std::cout << "========================================\n";
}

void printTestResult(const std::string& testName, bool passed) {
    std::cout << "[" << (passed ? "PASS" : "FAIL") << "] " << testName << "\n";
}

// ============================================================================
// Test 1: Simple Unreachable Code After Return
// ============================================================================
void testUnreachableAfterReturn() {
    printTestHeader("TEST 1: Unreachable Code After Return");

    // Create a function with dead code after return
    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Add instructions
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    // Return statement
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t0)));

    // Dead code after return (these should be removed)
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

    // Apply dead code elimination
    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should have removed 2 instructions
    bool passed = (removals == 2);
    printTestResult("Should remove 2 dead instructions after return", passed);

    // Verify only 2 instructions remain (add and return)
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool correctCount = (instructions.size() == 2);
    printTestResult("Should have exactly 2 instructions remaining", correctCount);

    std::cout << "\n";
}

// ============================================================================
// Test 2: Unreachable Code After Unconditional Jump
// ============================================================================
void testUnreachableAfterJump() {
    printTestHeader("TEST 2: Unreachable Code After Unconditional Jump");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Add a computation
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    // Unconditional jump
    block->addInstruction(std::make_unique<JumpInst>("target_block"));

    // Dead code after jump (should be removed)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("7", IROperand::OperandType::CONSTANT),
        IROperand("8", IROperand::OperandType::CONSTANT)));

    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &t2,
        IROperand("100", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (removals == 2);
    printTestResult("Should remove 2 dead instructions after jump", passed);

    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool correctCount = (instructions.size() == 2);
    printTestResult("Should have exactly 2 instructions remaining", correctCount);

    std::cout << "\n";
}

// ============================================================================
// Test 3: No Dead Code (Normal Control Flow)
// ============================================================================
void testNoDeadCode() {
    printTestHeader("TEST 3: No Dead Code (Normal Control Flow)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Add normal instructions without early return or jump
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand(t0),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    // Return at the end (normal case)
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t1)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should not remove any instructions
    bool passed = (removals == 0);
    printTestResult("Should not remove any instructions", passed);

    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool correctCount = (instructions.size() == 3);
    printTestResult("All 3 instructions should remain", correctCount);

    std::cout << "\n";
}

// ============================================================================
// Test 4: Dead Code with Label (Label Preserves Reachability)
// ============================================================================
void testDeadCodeWithLabel() {
    printTestHeader("TEST 4: Dead Code with Label (Label Preserves Reachability)");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Add computation
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    // Return statement
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t0)));

    // Label after return (makes subsequent code potentially reachable)
    block->addInstruction(std::make_unique<LabelInst>("loop_target"));

    // Code after label (reachable via jump to label)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("3", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should not remove any instructions (label makes code reachable)
    bool passed = (removals == 0);
    printTestResult("Should not remove code after label", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 5: Multiple Dead Instructions After Return
// ============================================================================
void testMultipleDeadInstructions() {
    printTestHeader("TEST 5: Multiple Dead Instructions After Return");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Return immediately
    block->addInstruction(std::make_unique<ReturnInst>(
        IROperand("42", IROperand::OperandType::CONSTANT)));

    // Many dead instructions
    for (int i = 0; i < 5; i++) {
        SSAValue temp("t", "int", i);
        block->addInstruction(std::make_unique<ArithmeticInst>(
            IROpcode::ADD, &temp,
            IROperand(std::to_string(i), IROperand::OperandType::CONSTANT),
            IROperand(std::to_string(i + 1), IROperand::OperandType::CONSTANT)));
    }

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should remove all 5 dead instructions
    bool passed = (removals == 5);
    printTestResult("Should remove all 5 dead instructions", passed);

    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool correctCount = (instructions.size() == 1);
    printTestResult("Only return should remain", correctCount);

    std::cout << "\n";
}

// ============================================================================
// Test 6: Dead Code in Multiple Basic Blocks
// ============================================================================
void testMultipleBasicBlocks() {
    printTestHeader("TEST 6: Dead Code in Multiple Basic Blocks");

    IRFunction func("test", "int");

    // Block 1: Has dead code after return
    auto block1 = std::make_unique<IRBasicBlock>("block1");
    block1->addInstruction(std::make_unique<ReturnInst>(
        IROperand("10", IROperand::OperandType::CONSTANT)));
    SSAValue t0("t", "int", 0);
    block1->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));
    func.addBasicBlock(std::move(block1));

    // Block 2: Has dead code after jump
    auto block2 = std::make_unique<IRBasicBlock>("block2");
    block2->addInstruction(std::make_unique<JumpInst>("target"));
    SSAValue t1("t", "int", 1);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("3", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));
    func.addBasicBlock(std::move(block2));

    // Block 3: No dead code
    auto block3 = std::make_unique<IRBasicBlock>("block3");
    SSAValue t2("t", "int", 2);
    block3->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));
    block3->addInstruction(std::make_unique<ReturnInst>(IROperand(t2)));
    func.addBasicBlock(std::move(block3));

    std::cout << "Before optimization:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.toString() << "\n";

    // Should remove 2 dead instructions (1 from block1, 1 from block2)
    bool passed = (removals == 2);
    printTestResult("Should remove 2 dead instructions across blocks", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 7: Jump Followed by Dead Code Then Label
// ============================================================================
void testJumpDeadCodeThenLabel() {
    printTestHeader("TEST 7: Jump Followed by Dead Code Then Label");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Unconditional jump
    block->addInstruction(std::make_unique<JumpInst>("else_block"));

    // Dead code after jump (should be removed)
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("3", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));

    // Label (marks reachable code)
    block->addInstruction(std::make_unique<LabelInst>("else_block"));

    // Code after label (reachable)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Should remove 2 dead instructions (between jump and label)
    bool passed = (removals == 2);
    printTestResult("Should remove dead code between jump and label", passed);

    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool correctCount = (instructions.size() == 3); // jump, label, sub
    printTestResult("Should have 3 instructions remaining", correctCount);

    std::cout << "\n";
}

// ============================================================================
// Test 8: Return with No Value (Void Function)
// ============================================================================
void testVoidReturn() {
    printTestHeader("TEST 8: Return with No Value (Void Function)");

    IRFunction func("test", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Some computation
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("10", IROperand::OperandType::CONSTANT)));

    // Void return (no value)
    block->addInstruction(std::make_unique<ReturnInst>());

    // Dead code after void return
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("2", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    bool passed = (removals == 1);
    printTestResult("Should remove dead code after void return", passed);

    std::cout << "\n";
}

// ============================================================================
// Test 9: Integration with Constant Folding
// ============================================================================
void testIntegrationWithConstantFolding() {
    printTestHeader("TEST 9: Integration with Constant Folding");

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Constant expression (will be folded)
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    // Return
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t0)));

    // Dead code after return
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("6", IROperand::OperandType::CONSTANT)));

    std::cout << "Before optimization:\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Run full optimization pipeline
    IROptimizer optimizer;
    optimizer.optimize(&func);

    std::cout << "After optimization:\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Verify: constant folding should have replaced ADD with MOVE
    const auto& instructions = func.getBasicBlocks()[0]->getInstructions();
    bool constantFolded = (instructions[0]->getOpcode() == IROpcode::MOVE);
    printTestResult("Should fold constant expression", constantFolded);

    // Verify: dead code removal should have removed instruction after return
    bool deadCodeRemoved = (instructions.size() == 2); // Only MOVE and RETURN
    printTestResult("Should remove dead code", deadCodeRemoved);

    std::cout << "\n";
}

// ============================================================================
// Test 10: Optimization Statistics
// ============================================================================
void testOptimizationStatistics() {
    printTestHeader("TEST 10: Optimization Statistics");

    IROptimizer optimizer;

    // Function 1: Has 2 dead instructions
    IRFunction func1("func1", "int");
    auto block1 = std::make_unique<IRBasicBlock>("entry");
    block1->addInstruction(std::make_unique<ReturnInst>(
        IROperand("10", IROperand::OperandType::CONSTANT)));
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

    optimizer.deadCodeEliminationPass(&func1);

    // Function 2: Has 1 dead instruction
    IRFunction func2("func2", "int");
    auto block2 = std::make_unique<IRBasicBlock>("entry");
    block2->addInstruction(std::make_unique<JumpInst>("target"));
    SSAValue t2("t", "int", 2);
    block2->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t2,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));
    func2.addBasicBlock(std::move(block2));

    optimizer.deadCodeEliminationPass(&func2);

    std::cout << "\n";
    optimizer.printOptimizationReport();

    // Should have 3 total removals
    bool passed = (optimizer.getDeadCodeEliminationCount() == 3);
    printTestResult("Should track total removals correctly", passed);

    std::cout << "\n";
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "DEAD CODE ELIMINATION TEST SUITE\n";
    std::cout << "========================================\n";
    std::cout << "User Story: As a compiler, I want to\n";
    std::cout << "remove unreachable code so that output\n";
    std::cout << "is cleaner.\n";
    std::cout << "\n";
    std::cout << "Acceptance Criteria:\n";
    std::cout << "- Code after unconditional jump removed\n";
    std::cout << "- Code after return removed\n";

    // Run all tests
    testUnreachableAfterReturn();
    testUnreachableAfterJump();
    testNoDeadCode();
    testDeadCodeWithLabel();
    testMultipleDeadInstructions();
    testMultipleBasicBlocks();
    testJumpDeadCodeThenLabel();
    testVoidReturn();
    testIntegrationWithConstantFolding();
    testOptimizationStatistics();

    std::cout << "========================================\n";
    std::cout << "All Dead Code Elimination Tests Complete!\n";
    std::cout << "========================================\n";

    return 0;
}
