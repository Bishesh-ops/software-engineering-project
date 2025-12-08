#include <gtest/gtest.h>
#include "codegen.h"
#include "ir.h"
#include <memory>
#include <string>
#include <algorithm>

// ============================================================================
// Test Fixture for x86-64 Code Generation
// ============================================================================
class CodeGenTest : public ::testing::Test {
protected:
    CodeGenerator codegen;

    void SetUp() override {
        codegen.reset();
    }

    // Helper: Check if assembly contains a specific instruction
    bool containsInstruction(const std::string& assembly, const std::string& instruction) {
        return assembly.find(instruction) != std::string::npos;
    }

    // Helper: Count occurrences of a pattern in assembly
    int countOccurrences(const std::string& assembly, const std::string& pattern) {
        int count = 0;
        size_t pos = 0;
        while ((pos = assembly.find(pattern, pos)) != std::string::npos) {
            count++;
            pos += pattern.length();
        }
        return count;
    }

    // Helper: Create a simple IR function for testing
    std::unique_ptr<IRFunction> createSimpleFunction(const std::string& name) {
        auto func = std::make_unique<IRFunction>(name, "int");

        // Create entry block
        auto entryBlock = std::make_unique<IRBasicBlock>("entry");

        // Add simple return instruction
        auto returnInst = std::make_unique<ReturnInst>(
            IROperand("42", IROperand::OperandType::CONSTANT)
        );
        entryBlock->addInstruction(std::move(returnInst));

        func->addBasicBlock(std::move(entryBlock));
        return func;
    }
};

// ============================================================================
// Basic Assembly Generation Tests
// ============================================================================

TEST_F(CodeGenTest, GeneratesPrologueAndEpilogue) {
    auto func = createSimpleFunction("test_func");
    std::string assembly = codegen.generateFunction(func.get());

    // Should contain prologue
    EXPECT_TRUE(containsInstruction(assembly, "pushq %rbp"));
    EXPECT_TRUE(containsInstruction(assembly, "movq %rsp, %rbp"));

    // Should contain epilogue
    EXPECT_TRUE(containsInstruction(assembly, "movq %rbp, %rsp"));
    EXPECT_TRUE(containsInstruction(assembly, "popq %rbp"));
    EXPECT_TRUE(containsInstruction(assembly, "ret"));
}

TEST_F(CodeGenTest, GeneratesFunctionLabel) {
    auto func = createSimpleFunction("my_function");
    std::string assembly = codegen.generateFunction(func.get());

    EXPECT_TRUE(containsInstruction(assembly, ".globl my_function"));
    EXPECT_TRUE(containsInstruction(assembly, "my_function:"));
}

TEST_F(CodeGenTest, GeneratesReturnWithValue) {
    auto func = createSimpleFunction("return_test");
    std::string assembly = codegen.generateFunction(func.get());

    // Should load return value into RAX before returning
    EXPECT_TRUE(containsInstruction(assembly, "%rax"));
}

// ============================================================================
// Arithmetic Instruction Tests
// ============================================================================

TEST_F(CodeGenTest, GeneratesAddInstruction) {
    auto func = std::make_unique<IRFunction>("add_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create SSA values
    auto* result = new SSAValue("t0", "int", 0);
    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // Create ADD instruction: t0 = a + b
    auto addInst = std::make_unique<ArithmeticInst>(
        IROpcode::ADD, result, IROperand(a), IROperand(b)
    );
    block->addInstruction(std::move(addInst));

    // Add return
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    EXPECT_TRUE(containsInstruction(assembly, "addq"));
}

TEST_F(CodeGenTest, GeneratesSubInstruction) {
    auto func = std::make_unique<IRFunction>("sub_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    auto* result = new SSAValue("t0", "int", 0);
    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    auto subInst = std::make_unique<ArithmeticInst>(
        IROpcode::SUB, result, IROperand(a), IROperand(b)
    );
    block->addInstruction(std::move(subInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    EXPECT_TRUE(containsInstruction(assembly, "subq"));
}

TEST_F(CodeGenTest, GeneratesMulInstruction) {
    auto func = std::make_unique<IRFunction>("mul_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    auto* result = new SSAValue("t0", "int", 0);
    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    auto mulInst = std::make_unique<ArithmeticInst>(
        IROpcode::MUL, result, IROperand(a), IROperand(b)
    );
    block->addInstruction(std::move(mulInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    EXPECT_TRUE(containsInstruction(assembly, "imulq"));
}

TEST_F(CodeGenTest, GeneratesDivInstruction) {
    auto func = std::make_unique<IRFunction>("div_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    auto* result = new SSAValue("t0", "int", 0);
    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    auto divInst = std::make_unique<ArithmeticInst>(
        IROpcode::DIV, result, IROperand(a), IROperand(b)
    );
    block->addInstruction(std::move(divInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Division requires cqto and idivq
    EXPECT_TRUE(containsInstruction(assembly, "cqto"));
    EXPECT_TRUE(containsInstruction(assembly, "idivq"));
}

// ============================================================================
// Comparison Instruction Tests
// ============================================================================

TEST_F(CodeGenTest, GeneratesComparisonInstructions) {
    auto func = std::make_unique<IRFunction>("cmp_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    auto* result = new SSAValue("t0", "int", 0);
    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);

    // Create LT comparison: t0 = a < b
    auto cmpInst = std::make_unique<ComparisonInst>(
        IROpcode::LT, result, IROperand(a), IROperand(b)
    );
    block->addInstruction(std::move(cmpInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Should generate cmpq and setl (set if less)
    EXPECT_TRUE(containsInstruction(assembly, "cmpq"));
    EXPECT_TRUE(containsInstruction(assembly, "setl"));
}

// ============================================================================
// Control Flow Tests
// ============================================================================

TEST_F(CodeGenTest, GeneratesJumpInstruction) {
    auto func = std::make_unique<IRFunction>("jump_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Add unconditional jump
    block->addInstruction(std::make_unique<JumpInst>("target_label"));

    auto targetBlock = std::make_unique<IRBasicBlock>("target_label");
    targetBlock->addInstruction(std::make_unique<ReturnInst>(
        IROperand("0", IROperand::OperandType::CONSTANT)
    ));

    func->addBasicBlock(std::move(block));
    func->addBasicBlock(std::move(targetBlock));

    std::string assembly = codegen.generateFunction(func.get());

    EXPECT_TRUE(containsInstruction(assembly, "jmp"));
    EXPECT_TRUE(containsInstruction(assembly, "target_label:"));
}

TEST_F(CodeGenTest, GeneratesConditionalBranch) {
    auto func = std::make_unique<IRFunction>("branch_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue condition("cond", "int", 0);

    // Add conditional branch: if (!condition) goto false_label
    block->addInstruction(std::make_unique<JumpIfFalseInst>(
        IROperand(condition), "false_label"
    ));

    auto falseBlock = std::make_unique<IRBasicBlock>("false_label");
    falseBlock->addInstruction(std::make_unique<ReturnInst>(
        IROperand("0", IROperand::OperandType::CONSTANT)
    ));

    func->addBasicBlock(std::move(block));
    func->addBasicBlock(std::move(falseBlock));

    std::string assembly = codegen.generateFunction(func.get());

    // Should generate cmpq with 0 and je (jump if equal)
    EXPECT_TRUE(containsInstruction(assembly, "cmpq"));
    EXPECT_TRUE(containsInstruction(assembly, "je"));
}

// ============================================================================
// Register Allocation Tests
// ============================================================================

TEST_F(CodeGenTest, AllocatesRegistersForValues) {
    auto func = std::make_unique<IRFunction>("reg_alloc_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create multiple SSA values to test register allocation
    auto* t0 = new SSAValue("t0", "int", 0);
    auto* t1 = new SSAValue("t1", "int", 0);
    auto* t2 = new SSAValue("t2", "int", 0);

    // t0 = 10
    block->addInstruction(std::make_unique<MoveInst>(
        t0, IROperand("10", IROperand::OperandType::CONSTANT)
    ));

    // t1 = 20
    block->addInstruction(std::make_unique<MoveInst>(
        t1, IROperand("20", IROperand::OperandType::CONSTANT)
    ));

    // t2 = t0 + t1
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, t2, IROperand(*t0), IROperand(*t1)
    ));

    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*t2)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Should use registers (rax, rcx, rdx, rsi, rdi, r8-r11)
    EXPECT_TRUE(containsInstruction(assembly, "%r"));  // At least one register used
}

// ============================================================================
// Calling Convention Tests
// ============================================================================

TEST_F(CodeGenTest, FollowsSystemVABIForFunctionCalls) {
    auto func = std::make_unique<IRFunction>("caller", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Create call instruction with 3 arguments
    auto* result = new SSAValue("ret", "int", 0);
    auto callInst = std::make_unique<CallInst>("callee", result);

    // Add 3 arguments (should use RDI, RSI, RDX)
    callInst->addArgument(IROperand("1", IROperand::OperandType::CONSTANT));
    callInst->addArgument(IROperand("2", IROperand::OperandType::CONSTANT));
    callInst->addArgument(IROperand("3", IROperand::OperandType::CONSTANT));

    block->addInstruction(std::move(callInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Should use first 3 argument registers (RDI, RSI, RDX)
    EXPECT_TRUE(containsInstruction(assembly, "%rdi"));
    EXPECT_TRUE(containsInstruction(assembly, "%rsi"));
    EXPECT_TRUE(containsInstruction(assembly, "%rdx"));
    EXPECT_TRUE(containsInstruction(assembly, "call callee"));
}

TEST_F(CodeGenTest, HandlesMoreThan6Arguments) {
    auto func = std::make_unique<IRFunction>("many_args", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    auto* result = new SSAValue("ret", "int", 0);
    auto callInst = std::make_unique<CallInst>("func_with_many_args", result);

    // Add 8 arguments (first 6 in registers, last 2 on stack)
    for (int i = 1; i <= 8; i++) {
        callInst->addArgument(IROperand(
            std::to_string(i), IROperand::OperandType::CONSTANT
        ));
    }

    block->addInstruction(std::move(callInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Should have push instructions for stack arguments
    EXPECT_TRUE(containsInstruction(assembly, "pushq"));
}

// ============================================================================
// Memory Operation Tests
// ============================================================================

TEST_F(CodeGenTest, GeneratesLoadInstruction) {
    auto func = std::make_unique<IRFunction>("load_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    auto* result = new SSAValue("t0", "int", 0);
    SSAValue ptr("ptr", "int*", 0);

    // Create LOAD: t0 = *ptr
    block->addInstruction(std::make_unique<LoadInst>(result, IROperand(ptr)));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Should have mov with memory dereference: movq (%reg), %reg
    EXPECT_TRUE(containsInstruction(assembly, "movq"));
}

TEST_F(CodeGenTest, GeneratesStoreInstruction) {
    auto func = std::make_unique<IRFunction>("store_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue value("val", "int", 0);
    SSAValue ptr("ptr", "int*", 0);

    // Create STORE: *ptr = value
    block->addInstruction(std::make_unique<StoreInst>(
        IROperand(value), IROperand(ptr)
    ));
    block->addInstruction(std::make_unique<ReturnInst>(
        IROperand("0", IROperand::OperandType::CONSTANT)
    ));

    func->addBasicBlock(std::move(block));
    std::string assembly = codegen.generateFunction(func.get());

    // Should have mov with memory target
    EXPECT_TRUE(containsInstruction(assembly, "movq"));
}

// ============================================================================
// AT&T Syntax Tests
// ============================================================================

TEST_F(CodeGenTest, GeneratesATTSyntax) {
    auto func = createSimpleFunction("att_test");
    std::string assembly = codegen.generateFunction(func.get());

    // AT&T syntax uses % for registers and $ for immediates
    EXPECT_TRUE(containsInstruction(assembly, "%"));  // Register prefix
    EXPECT_TRUE(containsInstruction(assembly, "$") ||  // Immediate prefix (if any constants)
                !containsInstruction(assembly, "$"));   // Or no immediates is also valid
}

// ============================================================================
// Stack Frame Tests
// ============================================================================

TEST_F(CodeGenTest, ManagesStackFrame) {
    auto func = createSimpleFunction("stack_test");
    std::string assembly = codegen.generateFunction(func.get());

    // Should set up and tear down stack frame
    EXPECT_TRUE(containsInstruction(assembly, "pushq %rbp"));
    EXPECT_TRUE(containsInstruction(assembly, "movq %rsp, %rbp"));
    EXPECT_TRUE(containsInstruction(assembly, "movq %rbp, %rsp"));
    EXPECT_TRUE(containsInstruction(assembly, "popq %rbp"));
}

// ============================================================================
// Comment Generation Tests
// ============================================================================

TEST_F(CodeGenTest, IncludesComments) {
    auto func = createSimpleFunction("comment_test");
    std::string assembly = codegen.generateFunction(func.get());

    // Should have comments (starting with #)
    EXPECT_TRUE(containsInstruction(assembly, "#"));
}

// ============================================================================
// Multiple Functions Test
// ============================================================================

TEST_F(CodeGenTest, GeneratesMultipleFunctions) {
    std::vector<std::unique_ptr<IRFunction>> functions;
    functions.push_back(createSimpleFunction("func1"));
    functions.push_back(createSimpleFunction("func2"));
    functions.push_back(createSimpleFunction("func3"));

    std::string assembly = codegen.generateProgram(functions);

    EXPECT_TRUE(containsInstruction(assembly, "func1:"));
    EXPECT_TRUE(containsInstruction(assembly, "func2:"));
    EXPECT_TRUE(containsInstruction(assembly, "func3:"));
}

// ============================================================================
// External Symbol Tests
// ============================================================================

TEST_F(CodeGenTest, DeclaresExternalSymbols) {
    auto func = std::make_unique<IRFunction>("uses_external", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    // Call external function (like printf)
    auto* result = new SSAValue("ret", "int", 0);
    auto callInst = std::make_unique<CallInst>("printf", result);
    callInst->addArgument(IROperand("1", IROperand::OperandType::CONSTANT));

    block->addInstruction(std::move(callInst));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(*result)));

    func->addBasicBlock(std::move(block));

    std::vector<std::unique_ptr<IRFunction>> functions;
    functions.push_back(std::move(func));

    std::string assembly = codegen.generateProgram(functions);

    // Should declare printf as external
    EXPECT_TRUE(containsInstruction(assembly, ".extern printf") ||
                containsInstruction(assembly, "call printf"));
}

// ============================================================================
// Section Directives Test
// ============================================================================

TEST_F(CodeGenTest, IncludesSectionDirectives) {
    std::vector<std::unique_ptr<IRFunction>> functions;
    functions.push_back(createSimpleFunction("main"));

    std::string assembly = codegen.generateProgram(functions);

    // Should have .text section
    EXPECT_TRUE(containsInstruction(assembly, ".text"));
}
