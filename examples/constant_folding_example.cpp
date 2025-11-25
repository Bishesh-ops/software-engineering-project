#include "ir.h"
#include "ir_optimizer.h"
#include <iostream>

// ============================================================================
// Constant Folding Optimization Example
// ============================================================================
// This example demonstrates how constant folding optimization works at the IR level
// User Story: As a compiler, I want to evaluate constant expressions at compile
// time so that runtime computation is reduced.

void printSeparator() {
    std::cout << "========================================\n";
}

void printHeader(const std::string& title) {
    printSeparator();
    std::cout << title << "\n";
    printSeparator();
}

// Example 1: Simple constant folding
void example1_SimpleConstantFolding() {
    printHeader("EXAMPLE 1: Simple Constant Folding");
    std::cout << "C Code: int x = 2 + 3;\n\n";

    // Create a function
    IRFunction func("example1", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Generate IR for: x = 2 + 3
    SSAValue x("x", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &x,
        IROperand("2", IROperand::OperandType::CONSTANT),
        IROperand("3", IROperand::OperandType::CONSTANT)));

    func.addBasicBlock(std::move(block));

    std::cout << "Before Optimization:\n";
    std::cout << func.toString() << "\n";

    // Apply constant folding
    IROptimizer optimizer;
    optimizer.constantFoldingPass(&func);

    std::cout << "After Optimization:\n";
    std::cout << func.toString() << "\n";
    std::cout << "Result: The ADD instruction is replaced with MOVE 5\n";
    std::cout << "Runtime computation eliminated!\n\n";
}

// Example 2: Multiplication by zero optimization
void example2_MultiplicationByZero() {
    printHeader("EXAMPLE 2: Multiplication by Zero");
    std::cout << "C Code: int result = 10 * 0;\n\n";

    IRFunction func("example2", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &result,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("0", IROperand::OperandType::CONSTANT)));

    func.addBasicBlock(std::move(block));

    std::cout << "Before Optimization:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    optimizer.constantFoldingPass(&func);

    std::cout << "After Optimization:\n";
    std::cout << func.toString() << "\n";
    std::cout << "Result: MUL replaced with MOVE 0\n";
    std::cout << "No multiplication at runtime!\n\n";
}

// Example 3: Complex expression with multiple constants
void example3_ComplexExpression() {
    printHeader("EXAMPLE 3: Complex Expression");
    std::cout << "C Code:\n";
    std::cout << "  int a = 5 + 10;\n";
    std::cout << "  int b = 20 / 4;\n";
    std::cout << "  int c = a + b;  // Not optimized (uses variables)\n\n";

    IRFunction func("example3", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // a = 5 + 10
    SSAValue a("a", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &a,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("10", IROperand::OperandType::CONSTANT)));

    // b = 20 / 4
    SSAValue b("b", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &b,
        IROperand("20", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));

    // c = a + b (uses variables, won't be optimized)
    SSAValue c("c", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &c,
        IROperand(a),
        IROperand(b)));

    func.addBasicBlock(std::move(block));

    std::cout << "Before Optimization:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After Optimization:\n";
    std::cout << func.toString() << "\n";
    std::cout << "Optimizations performed: " << optimizations << "\n";
    std::cout << "Note: Only pure constant operations are optimized\n";
    std::cout << "Variable operations remain unchanged\n\n";
}

// Example 4: Multiple basic blocks
void example4_MultipleBlocks() {
    printHeader("EXAMPLE 4: Optimization Across Basic Blocks");
    std::cout << "C Code:\n";
    std::cout << "  if (condition) {\n";
    std::cout << "    x = 100 + 200;  // Constant folding\n";
    std::cout << "  } else {\n";
    std::cout << "    x = 50 * 2;     // Constant folding\n";
    std::cout << "  }\n\n";

    IRFunction func("example4", "int");

    // Entry block
    auto entryBlock = std::make_unique<IRBasicBlock>("entry");
    SSAValue cond("cond", "bool", 0);
    entryBlock->addInstruction(std::make_unique<JumpIfFalseInst>(
        IROperand(cond), "else_block"));
    func.addBasicBlock(std::move(entryBlock));

    // Then block: x = 100 + 200
    auto thenBlock = std::make_unique<IRBasicBlock>("then_block");
    SSAValue x1("x", "int", 1);
    thenBlock->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &x1,
        IROperand("100", IROperand::OperandType::CONSTANT),
        IROperand("200", IROperand::OperandType::CONSTANT)));
    thenBlock->addInstruction(std::make_unique<JumpInst>("merge"));
    func.addBasicBlock(std::move(thenBlock));

    // Else block: x = 50 * 2
    auto elseBlock = std::make_unique<IRBasicBlock>("else_block");
    SSAValue x2("x", "int", 2);
    elseBlock->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &x2,
        IROperand("50", IROperand::OperandType::CONSTANT),
        IROperand("2", IROperand::OperandType::CONSTANT)));
    elseBlock->addInstruction(std::make_unique<JumpInst>("merge"));
    func.addBasicBlock(std::move(elseBlock));

    // Merge block with PHI
    auto mergeBlock = std::make_unique<IRBasicBlock>("merge");
    SSAValue x3("x", "int", 3);
    auto phi = std::make_unique<PhiInst>(&x3);
    phi->addIncoming(IROperand(x1), "then_block");
    phi->addIncoming(IROperand(x2), "else_block");
    mergeBlock->addInstruction(std::move(phi));
    mergeBlock->addInstruction(std::make_unique<ReturnInst>(IROperand(x3)));
    func.addBasicBlock(std::move(mergeBlock));

    std::cout << "Before Optimization:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After Optimization:\n";
    std::cout << func.toString() << "\n";
    std::cout << "Optimizations performed: " << optimizations << "\n";
    std::cout << "Both branches have constant folding applied\n\n";
}

// Example 5: Division by zero safety
void example5_DivisionByZero() {
    printHeader("EXAMPLE 5: Division by Zero Safety");
    std::cout << "C Code: int x = 10 / 0;  // Unsafe!\n\n";

    IRFunction func("example5", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &x,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("0", IROperand::OperandType::CONSTANT)));

    func.addBasicBlock(std::move(block));

    std::cout << "Before Optimization:\n";
    std::cout << func.toString() << "\n";

    IROptimizer optimizer;
    int optimizations = optimizer.constantFoldingPass(&func);

    std::cout << "After Optimization:\n";
    std::cout << func.toString() << "\n";
    std::cout << "Optimizations performed: " << optimizations << "\n";
    std::cout << "Note: Division by zero is NOT optimized (safety)\n";
    std::cout << "The runtime will handle this error appropriately\n\n";
}

// Example 6: Optimization statistics
void example6_OptimizationReport() {
    printHeader("EXAMPLE 6: Optimization Statistics");
    std::cout << "Demonstrating optimization tracking and reporting\n\n";

    IROptimizer optimizer;

    // Create multiple functions with constant operations
    for (int i = 0; i < 3; ++i) {
        IRFunction func("func" + std::to_string(i), "void");
        auto block = std::make_unique<IRBasicBlock>("entry");

        SSAValue result("result", "int", 0);
        block->addInstruction(std::make_unique<ArithmeticInst>(
            IROpcode::ADD, &result,
            IROperand(std::to_string(i * 10), IROperand::OperandType::CONSTANT),
            IROperand(std::to_string(i * 5), IROperand::OperandType::CONSTANT)));

        func.addBasicBlock(std::move(block));
        optimizer.constantFoldingPass(&func);
    }

    optimizer.printOptimizationReport();
}

int main() {
    printSeparator();
    std::cout << "CONSTANT FOLDING OPTIMIZATION EXAMPLES\n";
    printSeparator();
    std::cout << "\nUser Story: As a compiler, I want to evaluate constant\n";
    std::cout << "expressions at compile time so that runtime computation\n";
    std::cout << "is reduced.\n\n";

    example1_SimpleConstantFolding();
    example2_MultiplicationByZero();
    example3_ComplexExpression();
    example4_MultipleBlocks();
    example5_DivisionByZero();
    example6_OptimizationReport();

    printSeparator();
    std::cout << "All examples completed!\n";
    printSeparator();

    return 0;
}
