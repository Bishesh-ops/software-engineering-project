#include "ir.h"
#include "codegen.h"
#include "ir_optimizer.h"
#include <iostream>
#include <memory>

// ============================================================================
// x86-64 Code Generation Examples
// ============================================================================
// Educational examples demonstrating x86-64 assembly generation

void printSeparator() {
    std::cout << "\n========================================\n\n";
}

// ============================================================================
// Example 1: Simple Addition
// ============================================================================
void example1_SimpleAddition() {
    std::cout << "========================================\n";
    std::cout << "Example 1: Simple Addition\n";
    std::cout << "========================================\n\n";

    std::cout << "This example demonstrates basic code generation for addition.\n\n";

    IRFunction func("add_two_numbers", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue result("result", "int", 0);

    // result = a + b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, IROperand(a), IROperand(b)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    std::cout << "IR Code:\n";
    std::cout << "--------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    std::cout << "Generated x86-64 Assembly (AT&T Syntax):\n";
    std::cout << "---------------------------------------\n";
    std::cout << assembly << "\n";

    std::cout << "Explanation:\n";
    std::cout << "- Function prologue sets up stack frame\n";
    std::cout << "- addq performs 64-bit addition\n";
    std::cout << "- Result returned in %rax (System V ABI)\n";
    std::cout << "- Epilogue restores stack and returns\n";

    printSeparator();
}

// ============================================================================
// Example 2: Arithmetic Expression (a + b) * c
// ============================================================================
void example2_ArithmeticExpression() {
    std::cout << "========================================\n";
    std::cout << "Example 2: Arithmetic Expression\n";
    std::cout << "========================================\n\n";

    std::cout << "Computing (a + b) * c with multiple operations.\n\n";

    IRFunction func("compute_expression", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c("c", "int", 0);
    SSAValue t0("t", "int", 0);
    SSAValue result("result", "int", 1);

    // t0 = a + b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // result = t0 * c
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &result, IROperand(t0), IROperand(c)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    std::cout << "IR Code:\n";
    std::cout << "--------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    std::cout << "Generated x86-64 Assembly:\n";
    std::cout << "-------------------------\n";
    std::cout << assembly << "\n";

    std::cout << "Explanation:\n";
    std::cout << "- Two operations: ADD followed by MUL\n";
    std::cout << "- Intermediate result t_0 stored in register\n";
    std::cout << "- imulq performs signed multiplication\n";
    std::cout << "- Register allocator reuses registers when possible\n";

    printSeparator();
}

// ============================================================================
// Example 3: Comparison Operation
// ============================================================================
void example3_Comparison() {
    std::cout << "========================================\n";
    std::cout << "Example 3: Comparison Operation\n";
    std::cout << "========================================\n\n";

    std::cout << "This example demonstrates comparison code generation.\n\n";

    IRFunction func("is_less_than", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue cond("cond", "int", 0);

    // cond = x < y
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::LT, &cond, IROperand(x), IROperand(y)));

    // return cond
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(cond)));

    func.addBasicBlock(std::move(block));

    std::cout << "IR Code:\n";
    std::cout << "--------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    std::cout << "Generated x86-64 Assembly:\n";
    std::cout << "-------------------------\n";
    std::cout << assembly << "\n";

    std::cout << "Explanation:\n";
    std::cout << "- cmpq compares two 64-bit values\n";
    std::cout << "- setl sets byte to 1 if less-than condition true\n";
    std::cout << "- movzbq zero-extends 8-bit result to 64-bit\n";
    std::cout << "- Result (0 or 1) returned in %rax\n";

    printSeparator();
}

// ============================================================================
// Example 4: Constant Folding + Code Generation
// ============================================================================
void example4_OptimizedCode() {
    std::cout << "========================================\n";
    std::cout << "Example 4: Optimized Code Generation\n";
    std::cout << "========================================\n\n";

    std::cout << "This example shows code generation after optimization.\n\n";

    IRFunction func("compute_optimized", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue t0("t", "int", 0);
    SSAValue t1("t", "int", 1);
    SSAValue result("result", "int", 0);

    // t0 = 10 + 20 (will be folded to 30)
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0,
        IROperand("10", IROperand::OperandType::CONSTANT),
        IROperand("20", IROperand::OperandType::CONSTANT)));

    // t1 = 5 * 4 (will be folded to 20)
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1,
        IROperand("5", IROperand::OperandType::CONSTANT),
        IROperand("4", IROperand::OperandType::CONSTANT)));

    // result = t0 + t1
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, IROperand(t0), IROperand(t1)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    std::cout << "Original IR:\n";
    std::cout << "-----------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Optimize the IR
    IROptimizer optimizer;
    optimizer.optimize(&func);

    std::cout << "Optimized IR:\n";
    std::cout << "------------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Generate code from optimized IR
    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    std::cout << "Generated Assembly (from optimized IR):\n";
    std::cout << "---------------------------------------\n";
    std::cout << assembly << "\n";

    std::cout << "Explanation:\n";
    std::cout << "- Constants folded at compile time\n";
    std::cout << "- Fewer instructions in final assembly\n";
    std::cout << "- Code generator works on optimized IR\n";
    std::cout << "- Result: more efficient machine code\n";

    printSeparator();
}

// ============================================================================
// Example 5: Register Allocation Demonstration
// ============================================================================
void example5_RegisterAllocation() {
    std::cout << "========================================\n";
    std::cout << "Example 5: Register Allocation\n";
    std::cout << "========================================\n\n";

    std::cout << "This example demonstrates linear scan register allocation.\n\n";

    IRFunction func("many_values", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c("c", "int", 0);
    SSAValue d("d", "int", 0);
    SSAValue t0("t", "int", 0);
    SSAValue t1("t", "int", 1);
    SSAValue t2("t", "int", 2);
    SSAValue t3("t", "int", 3);

    // t0 = a + b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = c + d
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(c), IROperand(d)));

    // t2 = t0 * t1
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t2, IROperand(t0), IROperand(t1)));

    // t3 = t2 + t0
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t3, IROperand(t2), IROperand(t0)));

    // return t3
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t3)));

    func.addBasicBlock(std::move(block));

    std::cout << "IR Code:\n";
    std::cout << "--------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    // Show register allocation
    LinearScanAllocator allocator;
    allocator.buildLiveIntervals(&func);
    allocator.allocate();

    std::cout << "Register Allocation:\n";
    std::cout << "-------------------\n";
    allocator.printAllocation();
    std::cout << "\n";

    // Generate code
    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    std::cout << "Generated Assembly:\n";
    std::cout << "------------------\n";
    std::cout << assembly << "\n";

    std::cout << "Explanation:\n";
    std::cout << "- Linear scan allocator assigns registers efficiently\n";
    std::cout << "- Values with non-overlapping lifetimes share registers\n";
    std::cout << "- Reduces register pressure\n";
    std::cout << "- Spills to stack if registers exhausted\n";

    printSeparator();
}

// ============================================================================
// Example 6: AT&T Syntax Explained
// ============================================================================
void example6_ATTSyntax() {
    std::cout << "========================================\n";
    std::cout << "Example 6: AT&T Assembly Syntax\n";
    std::cout << "========================================\n\n";

    std::cout << "This example explains AT&T syntax conventions.\n\n";

    IRFunction func("syntax_demo", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue result("result", "int", 0);

    // result = x + 10
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, IROperand(x),
        IROperand("10", IROperand::OperandType::CONSTANT)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    std::cout << "IR Code:\n";
    std::cout << "--------\n";
    std::cout << func.getBasicBlocks()[0]->toString() << "\n";

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    std::cout << "Generated Assembly:\n";
    std::cout << "------------------\n";
    std::cout << assembly << "\n";

    std::cout << "AT&T Syntax Guide:\n";
    std::cout << "-----------------\n";
    std::cout << "1. Registers: Prefixed with %  (e.g., %rax, %rbx)\n";
    std::cout << "2. Immediates: Prefixed with $ (e.g., $10, $42)\n";
    std::cout << "3. Instruction format: OPCODE SOURCE, DEST\n";
    std::cout << "   - Example: addq $10, %rax  means %rax = %rax + 10\n";
    std::cout << "4. Size suffixes: b=byte, w=word, l=long, q=quad\n";
    std::cout << "   - addq = 64-bit add\n";
    std::cout << "   - movl = 32-bit move\n";
    std::cout << "5. Memory operands: OFFSET(%BASE)\n";
    std::cout << "   - -8(%rbp) = memory at rbp - 8\n\n";

    std::cout << "System V AMD64 ABI:\n";
    std::cout << "------------------\n";
    std::cout << "- Return value: %rax\n";
    std::cout << "- Integer args: %rdi, %rsi, %rdx, %rcx, %r8, %r9\n";
    std::cout << "- Caller-saved: %rax, %rcx, %rdx, %rsi, %rdi, %r8-r11\n";
    std::cout << "- Callee-saved: %rbx, %r12-r15, %rbp\n";
    std::cout << "- Stack pointer: %rsp\n";
    std::cout << "- Base pointer: %rbp\n";

    printSeparator();
}

// ============================================================================
// Main - Run All Examples
// ============================================================================

int main() {
    std::cout << "========================================\n";
    std::cout << "x86-64 CODE GENERATION EXAMPLES\n";
    std::cout << "========================================\n";
    std::cout << "\nUser Story:\n";
    std::cout << "As a compiler, I want to generate x86-64 machine code from IR\n";
    std::cout << "so that programs can execute on modern processors.\n";
    std::cout << "\nAcceptance Criteria:\n";
    std::cout << "- Generate valid x86-64 assembly in AT&T syntax\n";
    std::cout << "- Implement linear scan register allocation\n";
    std::cout << "- Follow System V AMD64 ABI conventions\n";
    std::cout << "- Support arithmetic, comparison, and control flow\n";
    std::cout << "\n";

    // Run all examples
    example1_SimpleAddition();
    example2_ArithmeticExpression();
    example3_Comparison();
    example4_OptimizedCode();
    example5_RegisterAllocation();
    example6_ATTSyntax();

    std::cout << "========================================\n";
    std::cout << "All Examples Complete!\n";
    std::cout << "========================================\n";
    std::cout << "\nKey Takeaways:\n";
    std::cout << "1. Code generator translates IR to x86-64 assembly\n";
    std::cout << "2. Linear scan register allocation is efficient\n";
    std::cout << "3. AT&T syntax used for GAS compatibility\n";
    std::cout << "4. System V ABI ensures interoperability\n";
    std::cout << "5. Optimization improves generated code quality\n";
    std::cout << "\nNext Steps:\n";
    std::cout << "- Assemble with: as -o output.o file.s\n";
    std::cout << "- Link with: ld output.o -o program\n";
    std::cout << "- Or use GCC: gcc file.s -o program\n";

    return 0;
}
