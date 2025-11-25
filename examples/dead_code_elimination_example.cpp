#include "ir.h"
#include "ir_optimizer.h"
#include <iostream>
#include <memory>

// ============================================================================
// Dead Code Elimination Examples
// ============================================================================
// This file demonstrates the dead code elimination optimization pass
// with educational examples showing unreachable code removal

void printSeparator() {
    std::cout << "\n========================================\n\n";
}

// ============================================================================
// Example 1: Basic Dead Code After Return
// ============================================================================
void example1_DeadCodeAfterReturn() {
    std::cout << "========================================\n";
    std::cout << "Example 1: Dead Code After Return\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows unreachable code after a return statement.\n";
    std::cout << "The optimizer will remove instructions that appear after return.\n\n";

    // Create function
    IRFunction func("calculateSum", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Compute result
    SSAValue sum("sum", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &sum,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    // Return the result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(sum)));

    // DEAD CODE: These instructions will never execute
    SSAValue deadVar1("x", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &deadVar1,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("6", IROperand::OperandType::CONSTANT)));

    SSAValue deadVar2("y", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &deadVar2,
        IROperand("100", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    std::cout << "Before Dead Code Elimination:\n";
    std::cout << "------------------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply optimization
    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After Dead Code Elimination:\n";
    std::cout << "-----------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << removals << " dead instructions removed\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- The two arithmetic operations after 'return' are unreachable\n";
    std::cout << "- They will never execute, so they can be safely removed\n";
    std::cout << "- This makes the generated code cleaner and smaller\n";

    printSeparator();
}

// ============================================================================
// Example 2: Dead Code After Unconditional Jump
// ============================================================================
void example2_DeadCodeAfterJump() {
    std::cout << "========================================\n";
    std::cout << "Example 2: Dead Code After Jump\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows unreachable code after an unconditional jump.\n";
    std::cout << "Instructions after 'jump' are unreachable and can be removed.\n\n";

    IRFunction func("conditionalLogic", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Some computation
    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &result,
        IROperand("7", IROperand::OperandType::CONSTANT),
        IROperand("8", IROperand::OperandType::CONSTANT)));

    // Unconditional jump to another block
    block->addInstruction(std::make_unique<JumpInst>("next_block"));

    // DEAD CODE: Will never execute because of unconditional jump above
    SSAValue deadVar("temp", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &deadVar,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    std::cout << "Before Dead Code Elimination:\n";
    std::cout << "------------------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply optimization
    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After Dead Code Elimination:\n";
    std::cout << "-----------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << removals << " dead instruction(s) removed\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- The unconditional 'jump' transfers control to 'next_block'\n";
    std::cout << "- Any code after the jump in the same block is unreachable\n";
    std::cout << "- The dead instruction is safely removed\n";

    printSeparator();
}

// ============================================================================
// Example 3: Labels Preserve Reachability
// ============================================================================
void example3_LabelPreservesReachability() {
    std::cout << "========================================\n";
    std::cout << "Example 3: Labels Preserve Reachability\n";
    std::cout << "========================================\n\n";

    std::cout << "This example demonstrates that labels mark potentially reachable code.\n";
    std::cout << "Code after a label is NOT removed, even if it appears after return.\n\n";

    IRFunction func("loopExample", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Initial computation
    SSAValue counter("i", "int", 0);
    block->addInstruction(std::make_unique<MoveInst>(
        &counter, IROperand("0", IROperand::OperandType::CONSTANT)));

    // Early return
    block->addInstruction(std::make_unique<ReturnInst>());

    // Label: Marks a potential jump target (e.g., from a loop)
    block->addInstruction(std::make_unique<LabelInst>("loop_body"));

    // Code after label is reachable via jumps to the label
    SSAValue increment("temp", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &increment,
        IROperand(counter),
        IROperand("1", IROperand::OperandType::CONSTANT)));

    std::cout << "Before Dead Code Elimination:\n";
    std::cout << "------------------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply optimization
    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After Dead Code Elimination:\n";
    std::cout << "-----------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << removals << " instruction(s) removed\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- Labels mark potential jump targets\n";
    std::cout << "- Code after a label may be reachable via jumps\n";
    std::cout << "- The optimizer preserves labels and code following them\n";
    std::cout << "- This ensures correctness for loops and conditional branches\n";

    printSeparator();
}

// ============================================================================
// Example 4: Dead Code Between Jump and Label
// ============================================================================
void example4_DeadCodeBetweenJumpAndLabel() {
    std::cout << "========================================\n";
    std::cout << "Example 4: Dead Code Between Jump and Label\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows dead code removal between a jump and a label.\n";
    std::cout << "Code between jump and label is unreachable and will be removed.\n\n";

    IRFunction func("complexControl", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Initial computation
    SSAValue value("val", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &value,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("10", IROperand::OperandType::CONSTANT)));

    // Jump to else block
    block->addInstruction(std::make_unique<JumpInst>("else_block"));

    // DEAD CODE: Between jump and label
    SSAValue dead1("x", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &dead1,
        IROperand("2", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    SSAValue dead2("y", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &dead2,
        IROperand("20", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    // Label: Marks start of reachable code
    block->addInstruction(std::make_unique<LabelInst>("else_block"));

    // Reachable code after label
    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &result,
        IROperand(value),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    std::cout << "Before Dead Code Elimination:\n";
    std::cout << "------------------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply optimization
    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After Dead Code Elimination:\n";
    std::cout << "-----------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << removals << " dead instruction(s) removed\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- Jump transfers control to 'else_block' label\n";
    std::cout << "- Instructions between jump and label are never executed\n";
    std::cout << "- These dead instructions are removed\n";
    std::cout << "- Code after the label remains (it's reachable)\n";

    printSeparator();
}

// ============================================================================
// Example 5: Void Return (No Return Value)
// ============================================================================
void example5_VoidReturn() {
    std::cout << "========================================\n";
    std::cout << "Example 5: Void Return\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows dead code elimination with void functions.\n";
    std::cout << "Even functions that don't return values can have dead code.\n\n";

    IRFunction func("printMessage", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Do some work
    SSAValue temp("temp", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &temp,
        IROperand("1", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    // Void return (no value)
    block->addInstruction(std::make_unique<ReturnInst>());

    // DEAD CODE: After void return
    SSAValue deadVar("x", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &deadVar,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    std::cout << "Before Dead Code Elimination:\n";
    std::cout << "------------------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply optimization
    IROptimizer optimizer;
    int removals = optimizer.deadCodeEliminationPass(&func);

    std::cout << "After Dead Code Elimination:\n";
    std::cout << "-----------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << removals << " dead instruction(s) removed\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- Even void functions can have dead code\n";
    std::cout << "- Code after 'return' is unreachable regardless of return type\n";
    std::cout << "- The optimizer removes it just like in value-returning functions\n";

    printSeparator();
}

// ============================================================================
// Example 6: Integration with Constant Folding
// ============================================================================
void example6_IntegrationWithConstantFolding() {
    std::cout << "========================================\n";
    std::cout << "Example 6: Integration with Constant Folding\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows how dead code elimination works with other\n";
    std::cout << "optimizations like constant folding in the optimization pipeline.\n\n";

    IRFunction func("optimizedFunction", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Constant expression (will be folded)
    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &result,
        IROperand("6", IROperand::OperandType::CONSTANT),
        IROperand("7", IROperand::OperandType::CONSTANT)));

    // Another constant expression (will be folded)
    SSAValue temp("temp", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &temp,
        IROperand("100", IROperand::OperandType::CONSTANT),
        IROperand("200", IROperand::OperandType::CONSTANT)));

    // Return
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    // DEAD CODE: Constant expression after return
    SSAValue deadConst("x", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &deadConst,
        IROperand("50", IROperand::OperandType::CONSTANT),
        IROperand("5", IROperand::OperandType::CONSTANT)));

    std::cout << "Before Optimization:\n";
    std::cout << "--------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    // Apply full optimization pipeline
    IROptimizer optimizer;
    optimizer.optimize(&func);

    std::cout << "After Full Optimization Pipeline:\n";
    std::cout << "----------------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "\nOptimization Report:\n";
    optimizer.printOptimizationReport();

    std::cout << "\nExplanation:\n";
    std::cout << "- Constant Folding: 3 constant expressions evaluated at compile time\n";
    std::cout << "- Dead Code Elimination: 1 unreachable instruction removed\n";
    std::cout << "- The optimizations work together to produce cleaner, faster code\n";
    std::cout << "- Final output has minimal instructions for maximum efficiency\n";

    printSeparator();
}

// ============================================================================
// Main - Run All Examples
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "DEAD CODE ELIMINATION EXAMPLES\n";
    std::cout << "========================================\n";
    std::cout << "\nUser Story:\n";
    std::cout << "As a compiler, I want to remove unreachable code\n";
    std::cout << "so that output is cleaner.\n";
    std::cout << "\nAcceptance Criteria:\n";
    std::cout << "- Code after unconditional jump removed\n";
    std::cout << "- Code after return removed\n";
    std::cout << "\n";

    // Run all examples
    example1_DeadCodeAfterReturn();
    example2_DeadCodeAfterJump();
    example3_LabelPreservesReachability();
    example4_DeadCodeBetweenJumpAndLabel();
    example5_VoidReturn();
    example6_IntegrationWithConstantFolding();

    std::cout << "========================================\n";
    std::cout << "All Examples Complete!\n";
    std::cout << "========================================\n";
    std::cout << "\nKey Takeaways:\n";
    std::cout << "1. Code after return or unconditional jump is unreachable\n";
    std::cout << "2. Labels mark potentially reachable code (jump targets)\n";
    std::cout << "3. Dead code elimination makes generated code cleaner\n";
    std::cout << "4. Works seamlessly with other optimization passes\n";
    std::cout << "5. Preserves program correctness while removing waste\n";

    return 0;
}
