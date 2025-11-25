#ifndef IR_OPTIMIZER_H
#define IR_OPTIMIZER_H

#include "ir.h"
#include <memory>
#include <vector>

// ============================================================================
// IR Optimizer - Performs optimization passes on SSA IR
// ============================================================================
// This module implements various optimization techniques at the IR level:
// - Constant Folding: Evaluate constant expressions at compile time
// - Dead Code Elimination: Remove unreachable or unused code (future)
// - Common Subexpression Elimination: Reuse computed values (future)

class IROptimizer
{
private:
    // Statistics tracking
    int constantFoldingCount;
    int deadCodeEliminationCount;
    int cseCount;

    // Helper: Check if an operand is a constant
    bool isConstantOperand(const IROperand& operand) const;

    // Helper: Extract integer value from constant operand
    bool getConstantValue(const IROperand& operand, int& value) const;

    // Helper: Evaluate a binary arithmetic operation on two constants
    bool evaluateConstantBinaryOp(IROpcode opcode, int left, int right, int& result) const;

    // Helper: Check if an arithmetic operation can be optimized
    bool canOptimizeArithmetic(const IRInstruction* inst) const;

public:
    IROptimizer();
    ~IROptimizer() = default;

    // ========================================================================
    // Optimization Passes
    // ========================================================================

    // Constant Folding: Evaluate constant expressions at compile time
    // Example: t0 = add 2, 3  →  t0 = move 5
    // Example: t1 = mul 10, 0  →  t1 = move 0
    // Returns: Number of instructions optimized
    int constantFoldingPass(IRFunction* function);

    // Apply constant folding to a single basic block
    // Returns: Number of instructions optimized in this block
    int constantFoldingPass(IRBasicBlock* block);

    // Dead Code Elimination: Remove instructions that compute unused values
    // (Future implementation)
    int deadCodeEliminationPass(IRFunction* function);

    // Common Subexpression Elimination: Reuse previously computed values
    // (Future implementation)
    int commonSubexpressionEliminationPass(IRFunction* function);

    // ========================================================================
    // Optimization Pipeline
    // ========================================================================

    // Run all enabled optimization passes on a function
    // This is the main entry point for optimization
    void optimize(IRFunction* function);

    // Run all enabled optimization passes on a vector of functions
    void optimize(std::vector<std::unique_ptr<IRFunction>>& functions);

    // ========================================================================
    // Statistics and Configuration
    // ========================================================================

    // Get optimization statistics
    int getConstantFoldingCount() const { return constantFoldingCount; }
    int getDeadCodeEliminationCount() const { return deadCodeEliminationCount; }
    int getCSECount() const { return cseCount; }

    // Reset statistics
    void resetStatistics();

    // Print optimization report
    void printOptimizationReport() const;
};

#endif // IR_OPTIMIZER_H
