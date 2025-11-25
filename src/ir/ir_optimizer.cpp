#include "ir_optimizer.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

// ============================================================================
// Constructor
// ============================================================================
IROptimizer::IROptimizer()
    : constantFoldingCount(0), deadCodeEliminationCount(0), cseCount(0)
{
}

// ============================================================================
// Helper Methods
// ============================================================================

bool IROptimizer::isConstantOperand(const IROperand& operand) const
{
    return operand.isConstant();
}

bool IROptimizer::getConstantValue(const IROperand& operand, int& value) const
{
    if (!operand.isConstant()) {
        return false;
    }

    try {
        // Parse the constant string to integer
        value = std::stoi(operand.getConstant());
        return true;
    } catch (const std::exception& e) {
        // Could not parse as integer (might be float, string, etc.)
        return false;
    }
}

bool IROptimizer::evaluateConstantBinaryOp(IROpcode opcode, int left, int right, int& result) const
{
    switch (opcode) {
        case IROpcode::ADD:
            result = left + right;
            return true;

        case IROpcode::SUB:
            result = left - right;
            return true;

        case IROpcode::MUL:
            result = left * right;
            return true;

        case IROpcode::DIV:
            // Avoid division by zero
            if (right == 0) {
                return false;
            }
            result = left / right;
            return true;

        case IROpcode::MOD:
            // Avoid modulo by zero
            if (right == 0) {
                return false;
            }
            result = left % right;
            return true;

        default:
            // Not an arithmetic operation we can optimize
            return false;
    }
}

bool IROptimizer::canOptimizeArithmetic(const IRInstruction* inst) const
{
    // Check if this is an arithmetic instruction
    IROpcode opcode = inst->getOpcode();
    if (opcode != IROpcode::ADD && opcode != IROpcode::SUB &&
        opcode != IROpcode::MUL && opcode != IROpcode::DIV &&
        opcode != IROpcode::MOD) {
        return false;
    }

    // Check if it has a result (destination)
    if (inst->getResult() == nullptr) {
        return false;
    }

    // Check if both operands are constants
    const auto& operands = inst->getOperands();
    if (operands.size() != 2) {
        return false;
    }

    return isConstantOperand(operands[0]) && isConstantOperand(operands[1]);
}

// ============================================================================
// Constant Folding Pass (Basic Block Level)
// ============================================================================

int IROptimizer::constantFoldingPass(IRBasicBlock* block)
{
    int optimizationsPerformed = 0;
    auto& instructions = const_cast<std::vector<std::unique_ptr<IRInstruction>>&>(
        block->getInstructions());

    // Iterate through instructions and look for constant arithmetic operations
    for (size_t i = 0; i < instructions.size(); ++i) {
        IRInstruction* inst = instructions[i].get();

        // Check if this instruction can be optimized
        if (!canOptimizeArithmetic(inst)) {
            continue;
        }

        // Get operands
        const auto& operands = inst->getOperands();
        int leftValue, rightValue, resultValue;

        // Extract constant values
        if (!getConstantValue(operands[0], leftValue) ||
            !getConstantValue(operands[1], rightValue)) {
            continue;
        }

        // Evaluate the operation
        if (!evaluateConstantBinaryOp(inst->getOpcode(), leftValue, rightValue, resultValue)) {
            continue;
        }

        // Create a new MOVE instruction with the computed constant
        // Format: result = move <computed_constant>
        SSAValue* result = inst->getResult();
        std::string constantStr = std::to_string(resultValue);
        IROperand constantOperand(constantStr, IROperand::OperandType::CONSTANT);

        // Replace the arithmetic instruction with a MOVE instruction
        instructions[i] = std::make_unique<MoveInst>(result, constantOperand);

        optimizationsPerformed++;
    }

    return optimizationsPerformed;
}

// ============================================================================
// Constant Folding Pass (Function Level)
// ============================================================================

int IROptimizer::constantFoldingPass(IRFunction* function)
{
    int totalOptimizations = 0;

    // Apply constant folding to each basic block in the function
    auto& blocks = const_cast<std::vector<std::unique_ptr<IRBasicBlock>>&>(
        function->getBasicBlocks());

    for (auto& block : blocks) {
        int blockOptimizations = constantFoldingPass(block.get());
        totalOptimizations += blockOptimizations;
    }

    // Update statistics
    constantFoldingCount += totalOptimizations;

    return totalOptimizations;
}

// ============================================================================
// Dead Code Elimination Pass
// ============================================================================
// Removes unreachable code after unconditional jumps and return statements
// User Story: As a compiler, I want to remove unreachable code so that output
//             is cleaner
// Acceptance Criteria:
// - Code after unconditional jump removed
// - Code after return removed

int IROptimizer::deadCodeEliminationPass(IRFunction* function)
{
    int totalRemovals = 0;

    // Iterate through all basic blocks in the function
    auto& blocks = const_cast<std::vector<std::unique_ptr<IRBasicBlock>>&>(
        function->getBasicBlocks());

    for (auto& block : blocks) {
        auto& instructions = const_cast<std::vector<std::unique_ptr<IRInstruction>>&>(
            block->getInstructions());

        // Find the position of the first unconditional jump or return
        size_t terminatorPos = instructions.size();
        bool foundTerminator = false;

        for (size_t i = 0; i < instructions.size(); ++i) {
            IROpcode opcode = instructions[i]->getOpcode();

            // Check for unconditional jump or return
            if (opcode == IROpcode::JUMP || opcode == IROpcode::RETURN) {
                terminatorPos = i;
                foundTerminator = true;
                break;
            }
        }

        // If we found a terminator and there are instructions after it
        if (foundTerminator && terminatorPos + 1 < instructions.size()) {
            // Count dead instructions (excluding labels, as they may be jump targets)
            size_t deadCodeStart = terminatorPos + 1;
            size_t deadCodeCount = 0;

            // Check if the next instruction after terminator is a label
            // If so, we stop - code after a label is potentially reachable
            if (deadCodeStart < instructions.size()) {
                IROpcode nextOpcode = instructions[deadCodeStart]->getOpcode();
                if (nextOpcode == IROpcode::LABEL) {
                    // Code after label is reachable, don't remove
                    continue;
                }
            }

            // Count and remove dead instructions until we hit a label or end
            for (size_t i = deadCodeStart; i < instructions.size(); ++i) {
                IROpcode opcode = instructions[i]->getOpcode();

                // Stop if we encounter a label (it's a potential jump target)
                if (opcode == IROpcode::LABEL) {
                    break;
                }

                deadCodeCount++;
            }

            // Remove the dead instructions
            if (deadCodeCount > 0) {
                instructions.erase(
                    instructions.begin() + deadCodeStart,
                    instructions.begin() + deadCodeStart + deadCodeCount
                );
                totalRemovals += deadCodeCount;
            }
        }
    }

    // Update statistics
    deadCodeEliminationCount += totalRemovals;

    return totalRemovals;
}

// ============================================================================
// Common Subexpression Elimination Pass (Future)
// ============================================================================

int IROptimizer::commonSubexpressionEliminationPass(IRFunction* function)
{
    // TODO: Implement common subexpression elimination
    // This will identify repeated computations and reuse previous results
    (void)function;  // Suppress unused parameter warning
    return 0;
}

// ============================================================================
// Optimization Pipeline
// ============================================================================

void IROptimizer::optimize(IRFunction* function)
{
    // Run constant folding pass
    constantFoldingPass(function);

    // Run dead code elimination pass
    deadCodeEliminationPass(function);

    // Future passes:
    // commonSubexpressionEliminationPass(function);
}

void IROptimizer::optimize(std::vector<std::unique_ptr<IRFunction>>& functions)
{
    for (auto& function : functions) {
        optimize(function.get());
    }
}

// ============================================================================
// Statistics
// ============================================================================

void IROptimizer::resetStatistics()
{
    constantFoldingCount = 0;
    deadCodeEliminationCount = 0;
    cseCount = 0;
}

void IROptimizer::printOptimizationReport() const
{
    std::cout << "========================================\n";
    std::cout << "IR Optimization Report\n";
    std::cout << "========================================\n";
    std::cout << "Constant Folding:              " << constantFoldingCount << " optimizations\n";
    std::cout << "Dead Code Elimination:         " << deadCodeEliminationCount << " optimizations\n";
    std::cout << "Common Subexpression Elim.:    " << cseCount << " optimizations\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Total Optimizations:           "
              << (constantFoldingCount + deadCodeEliminationCount + cseCount) << "\n";
    std::cout << "========================================\n";
}
