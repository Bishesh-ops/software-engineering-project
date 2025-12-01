#include "ir.h"
#include "ir_optimizer.h"
#include <iostream>
#include <memory>

// ============================================================================
// Common Subexpression Elimination Examples
// ============================================================================
// Educational examples demonstrating CSE optimization

void printSeparator() {
    std::cout << "\n========================================\n\n";
}

// ============================================================================
// Example 1: Basic CSE (a + b repeated)
// ============================================================================
void example1_BasicCSE() {
    std::cout << "========================================\n";
    std::cout << "Example 1: Basic Common Subexpression\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows elimination of a repeated expression.\n";
    std::cout << "When 'a + b' is computed twice, the second computation is replaced\n";
    std::cout << "with a copy of the first result.\n\n";

    IRFunction func("calculate", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // First computation: result1 = a + b
    SSAValue result1("result", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result1, IROperand(a), IROperand(b)));

    // Second computation: result2 = a + b (DUPLICATE!)
    SSAValue result2("result", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result2, IROperand(a), IROperand(b)));

    std::cout << "Before CSE:\n";
    std::cout << "-----------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << "----------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << optimizations << " subexpression(s) eliminated\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- The first 'a + b' is computed and stored in result_1\n";
    std::cout << "- The second 'a + b' is redundant - we already know the answer!\n";
    std::cout << "- Replaced with: result_2 = result_1 (just copy the value)\n";
    std::cout << "- Saves one ADD operation at runtime\n";

    printSeparator();
}

// ============================================================================
// Example 2: Multiple Common Subexpressions
// ============================================================================
void example2_MultipleCSE() {
    std::cout << "========================================\n";
    std::cout << "Example 2: Multiple Common Subexpressions\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows multiple opportunities for CSE in one block.\n\n";

    IRFunction func("compute", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);

    // t0 = x * y
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t0, IROperand(x), IROperand(y)));

    // Some unique computation
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(x),
        IROperand("1", IROperand::OperandType::CONSTANT)));

    // t2 = x * y (duplicate #1)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2, IROperand(x), IROperand(y)));

    // t3 = x * y (duplicate #2)
    SSAValue t3("t", "int", 3);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t3, IROperand(x), IROperand(y)));

    std::cout << "Before CSE:\n";
    std::cout << "-----------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << "----------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << optimizations << " subexpression(s) eliminated\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- First 'x * y' computed once and stored\n";
    std::cout << "- Both subsequent 'x * y' operations eliminated\n";
    std::cout << "- Replaced with simple MOVE operations\n";
    std::cout << "- Saves two expensive MUL operations!\n";

    printSeparator();
}

// ============================================================================
// Example 3: CSE with Different Operations (No Optimization)
// ============================================================================
void example3_DifferentOps() {
    std::cout << "========================================\n";
    std::cout << "Example 3: Different Operations\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows that CSE only applies to identical expressions.\n";
    std::cout << "Different operations are NOT considered common subexpressions.\n\n";

    IRFunction func("mixed", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // t0 = a + b
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = a - b (different operation)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &t1, IROperand(a), IROperand(b)));

    // t2 = a * b (different operation)
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2, IROperand(a), IROperand(b)));

    std::cout << "Before CSE:\n";
    std::cout << "-----------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << "----------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << optimizations << " subexpression(s) eliminated\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- Even though all use 'a' and 'b', the operations differ\n";
    std::cout << "- a + b ≠ a - b ≠ a * b\n";
    std::cout << "- No common subexpressions found\n";
    std::cout << "- All operations preserved (correct!)\n";

    printSeparator();
}

// ============================================================================
// Example 4: CSE with Comparisons
// ============================================================================
void example4_Comparisons() {
    std::cout << "========================================\n";
    std::cout << "Example 4: CSE with Comparison Operations\n";
    std::cout << "========================================\n\n";

    std::cout << "CSE also works with comparison operations!\n\n";

    IRFunction func("compare", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);

    // cond1 = x < y
    SSAValue cond1("cond", "int", 1);
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::LT, &cond1, IROperand(x), IROperand(y)));

    // Some other operation
    SSAValue temp("temp", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &temp, IROperand(x), IROperand(y)));

    // cond2 = x < y (duplicate comparison!)
    SSAValue cond2("cond", "int", 2);
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::LT, &cond2, IROperand(x), IROperand(y)));

    std::cout << "Before CSE:\n";
    std::cout << "-----------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << "----------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << optimizations << " subexpression(s) eliminated\n";
    std::cout << "\nExplanation:\n";
    std::cout << "- Comparison operations are pure (no side effects)\n";
    std::cout << "- Safe to eliminate redundant comparisons\n";
    std::cout << "- Second 'x < y' replaced with copy of first result\n";

    printSeparator();
}

// ============================================================================
// Example 5: Integration with Full Optimization Pipeline
// ============================================================================
void example5_FullPipeline() {
    std::cout << "========================================\n";
    std::cout << "Example 5: Full Optimization Pipeline\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows CSE working with other optimizations.\n\n";

    IRFunction func("optimized", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // t0 = 10 + 20 (will be constant folded to 30)
    SSAValue t0("t", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    // t1 = 10 + 20 (duplicate - will also be folded, then CSE'd)
    SSAValue t1("t", "int", 1);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    // t2 = t0 + t1
    SSAValue t2("t", "int", 2);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t2, IROperand(t0), IROperand(t1)));

    // Return statement
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t2)));

    // Dead code after return
    SSAValue dead("dead", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &dead,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("6", IROperand::OperandType::CONSTANT)));

    std::cout << "Before Optimization:\n";
    std::cout << "--------------------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    optimizer.optimize(&func);

    std::cout << "After Full Optimization Pipeline:\n";
    std::cout << "----------------------------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "\nOptimization Report:\n";
    optimizer.printOptimizationReport();

    std::cout << "\nExplanation:\n";
    std::cout << "1. Constant Folding: 10+20 → 30 (both occurrences)\n";
    std::cout << "2. CSE: After folding, both are 'move 30', second eliminated\n";
    std::cout << "3. Dead Code Elimination: Code after return removed\n";
    std::cout << "4. Result: Clean, efficient IR with minimal instructions\n";

    printSeparator();
}

// ============================================================================
// Example 6: Real-World Scenario
// ============================================================================
void example6_RealWorld() {
    std::cout << "========================================\n";
    std::cout << "Example 6: Real-World Scenario\n";
    std::cout << "========================================\n\n";

    std::cout << "Imagine computing area and perimeter of a rectangle.\n";
    std::cout << "Both formulas use 'length * width'.\n\n";

    IRFunction func("rectangle_calcs", "void");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue length("length", "int", 0);
    SSAValue width("width", "int", 0);

    // area = length * width
    SSAValue area("area", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &area, IROperand(length), IROperand(width)));

    // For perimeter, we also need the product (maybe for validation)
    // product = length * width (DUPLICATE!)
    SSAValue product("product", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &product, IROperand(length), IROperand(width)));

    // perimeter = 2 * (length + width)
    SSAValue sum("sum", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &sum, IROperand(length), IROperand(width)));

    SSAValue perimeter("perimeter", "int", 0);
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &perimeter, IROperand(sum),
        IROperand("2", IROperand::OperandType::CONSTANT)));

    std::cout << "Before CSE:\n";
    std::cout << "-----------\n";
    std::cout << block->toString() << "\n";

    func.addBasicBlock(std::move(block));

    IROptimizer optimizer;
    int optimizations = optimizer.commonSubexpressionEliminationPass(&func);

    std::cout << "After CSE:\n";
    std::cout << "----------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    std::cout << "Optimizations: " << optimizations << " subexpression(s) eliminated\n";
    std::cout << "\nReal-World Benefit:\n";
    std::cout << "- Multiplication is expensive (multiple CPU cycles)\n";
    std::cout << "- CSE eliminates redundant 'length * width' computation\n";
    std::cout << "- Faster execution with identical results\n";
    std::cout << "- This pattern appears frequently in real code!\n";

    printSeparator();
}

// ============================================================================
// Main - Run All Examples
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "COMMON SUBEXPRESSION ELIMINATION EXAMPLES\n";
    std::cout << "========================================\n";
    std::cout << "\nUser Story:\n";
    std::cout << "As a compiler, I want to eliminate redundant computations\n";
    std::cout << "so that performance is improved.\n";
    std::cout << "\nAcceptance Criteria:\n";
    std::cout << "- t0 = a + b; t1 = a + b;\n";
    std::cout << "  → t0 = a + b; t1 = t0;\n";
    std::cout << "\n";

    // Run all examples
    example1_BasicCSE();
    example2_MultipleCSE();
    example3_DifferentOps();
    example4_Comparisons();
    example5_FullPipeline();
    example6_RealWorld();

    std::cout << "========================================\n";
    std::cout << "All Examples Complete!\n";
    std::cout << "========================================\n";
    std::cout << "\nKey Takeaways:\n";
    std::cout << "1. CSE eliminates redundant computations\n";
    std::cout << "2. Only identical expressions are eliminated\n";
    std::cout << "3. Works with arithmetic and comparison operations\n";
    std::cout << "4. Integrates seamlessly with other optimizations\n";
    std::cout << "5. Significant performance benefit in real code\n";

    return 0;
}
