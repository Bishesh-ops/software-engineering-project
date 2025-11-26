#include "codegen.h"
#include "ir.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <string>

// ============================================================================
// Test Framework
// ============================================================================

int totalTests = 0;
int passedTests = 0;

void reportTest(const std::string& testName, bool passed)
{
    totalTests++;
    if (passed) {
        passedTests++;
        std::cout << "[PASS] " << testName << "\n";
    } else {
        std::cout << "[FAIL] " << testName << "\n";
    }
}

// ============================================================================
// Test 1: Linear Scan Allocator - Build Live Intervals
// ============================================================================
void test_LinearScanBuildIntervals()
{
    std::string testName = "LinearScan: Build Live Intervals";

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue result("result", "int", 0);

    // result = a + b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, IROperand(a), IROperand(b)));

    func.addBasicBlock(std::move(block));

    LinearScanAllocator allocator;
    allocator.buildLiveIntervals(&func);

    // After building intervals, allocation should work
    allocator.allocate();

    // Check that result got a register
    X86Register reg = allocator.getRegister(&result);
    bool passed = (reg != X86Register::NONE);

    reportTest(testName, passed);
}

// ============================================================================
// Test 2: Linear Scan Allocator - Simple Allocation
// ============================================================================
void test_LinearScanSimpleAllocation()
{
    std::string testName = "LinearScan: Simple Register Allocation";

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue z("z", "int", 0);

    // z = x + y
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &z, IROperand(x), IROperand(y)));

    func.addBasicBlock(std::move(block));

    LinearScanAllocator allocator;
    allocator.buildLiveIntervals(&func);
    allocator.allocate();

    // All values should get registers (only 3 values, plenty of registers)
    bool passed = (allocator.getRegister(&z) != X86Register::NONE);

    reportTest(testName, passed);
}

// ============================================================================
// Test 3: Linear Scan Allocator - Multiple Values
// ============================================================================
void test_LinearScanMultipleValues()
{
    std::string testName = "LinearScan: Multiple Values";

    IRFunction func("test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c("c", "int", 0);
    SSAValue d("d", "int", 0);
    SSAValue t0("t", "int", 0);
    SSAValue t1("t", "int", 1);
    SSAValue t2("t", "int", 2);

    // t0 = a + b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = c + d
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t1, IROperand(c), IROperand(d)));

    // t2 = t0 + t1
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t2, IROperand(t0), IROperand(t1)));

    func.addBasicBlock(std::move(block));

    LinearScanAllocator allocator;
    allocator.buildLiveIntervals(&func);
    allocator.allocate();

    // All results should get registers
    bool passed = (allocator.getRegister(&t0) != X86Register::NONE) &&
                  (allocator.getRegister(&t1) != X86Register::NONE) &&
                  (allocator.getRegister(&t2) != X86Register::NONE);

    reportTest(testName, passed);
}

// ============================================================================
// Test 4: Code Generator - Simple Arithmetic
// ============================================================================
void test_CodeGenSimpleArithmetic()
{
    std::string testName = "CodeGen: Simple Arithmetic (a + b)";

    IRFunction func("add_numbers", "int");
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

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify assembly contains key elements
    bool hasFunction = assembly.find("add_numbers:") != std::string::npos;
    bool hasPrologue = assembly.find("pushq %rbp") != std::string::npos;
    bool hasAdd = assembly.find("addq") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasFunction && hasPrologue && hasAdd && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 5: Code Generator - Subtraction
// ============================================================================
void test_CodeGenSubtraction()
{
    std::string testName = "CodeGen: Subtraction (x - y)";

    IRFunction func("subtract", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue result("result", "int", 0);

    // result = x - y
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::SUB, &result, IROperand(x), IROperand(y)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify subtraction instruction
    bool hasSub = assembly.find("subq") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasSub && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 6: Code Generator - Multiplication
// ============================================================================
void test_CodeGenMultiplication()
{
    std::string testName = "CodeGen: Multiplication (a * b)";

    IRFunction func("multiply", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue result("result", "int", 0);

    // result = a * b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &result, IROperand(a), IROperand(b)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify multiplication instruction
    bool hasMul = assembly.find("imulq") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasMul && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 7: Code Generator - Comparison
// ============================================================================
void test_CodeGenComparison()
{
    std::string testName = "CodeGen: Comparison (x < y)";

    IRFunction func("compare", "int");
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

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify comparison instructions
    bool hasCmp = assembly.find("cmpq") != std::string::npos;
    bool hasSet = assembly.find("setl") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasCmp && hasSet && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 8: Code Generator - Move Instruction
// ============================================================================
void test_CodeGenMove()
{
    std::string testName = "CodeGen: Move Instruction";

    IRFunction func("move_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);

    // y = x
    block->addInstruction(std::make_unique<MoveInst>(
        &y, IROperand(x)));

    // return y
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(y)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify move instruction
    bool hasMove = assembly.find("movq") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasMove && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 9: Code Generator - Constant Loading
// ============================================================================
void test_CodeGenConstant()
{
    std::string testName = "CodeGen: Constant Loading";

    IRFunction func("load_constant", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue result("result", "int", 0);

    // result = 42
    block->addInstruction(std::make_unique<MoveInst>(
        &result, IROperand("42", IROperand::OperandType::CONSTANT)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify constant is loaded with $ prefix (AT&T syntax)
    bool hasConstant = assembly.find("$42") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasConstant && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 10: Code Generator - Multiple Instructions
// ============================================================================
void test_CodeGenMultipleInstructions()
{
    std::string testName = "CodeGen: Multiple Instructions";

    IRFunction func("complex", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c("c", "int", 0);
    SSAValue t0("t", "int", 0);
    SSAValue t1("t", "int", 1);

    // t0 = a + b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &t0, IROperand(a), IROperand(b)));

    // t1 = t0 * c
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MUL, &t1, IROperand(t0), IROperand(c)));

    // return t1
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(t1)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify both operations present
    bool hasAdd = assembly.find("addq") != std::string::npos;
    bool hasMul = assembly.find("imulq") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasAdd && hasMul && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 11: Code Generator - Prologue and Epilogue
// ============================================================================
void test_CodeGenPrologueEpilogue()
{
    std::string testName = "CodeGen: Function Prologue and Epilogue";

    IRFunction func("test_func", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<MoveInst>(
        &result, IROperand("0", IROperand::OperandType::CONSTANT)));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify standard prologue/epilogue
    bool hasPushRBP = assembly.find("pushq %rbp") != std::string::npos;
    bool hasMovRSPtoRBP = assembly.find("movq %rsp, %rbp") != std::string::npos;
    bool hasMovRBPtoRSP = assembly.find("movq %rbp, %rsp") != std::string::npos;
    bool hasPopRBP = assembly.find("popq %rbp") != std::string::npos;
    bool hasRet = assembly.find("ret") != std::string::npos;

    bool passed = hasPushRBP && hasMovRSPtoRBP && hasMovRBPtoRSP && hasPopRBP && hasRet;

    reportTest(testName, passed);
}

// ============================================================================
// Test 12: Code Generator - AT&T Syntax Validation
// ============================================================================
void test_CodeGenATTSyntax()
{
    std::string testName = "CodeGen: AT&T Syntax Validation";

    IRFunction func("syntax_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue result("result", "int", 0);

    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, IROperand(a), IROperand(b)));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify AT&T syntax characteristics:
    // - Registers prefixed with %
    // - Constants prefixed with $
    // - Source before destination
    bool hasPercentRegisters = assembly.find("%r") != std::string::npos;
    bool passed = hasPercentRegisters;

    reportTest(testName, passed);
}

// ============================================================================
// Test 13: Code Generator - Register Usage
// ============================================================================
void test_CodeGenRegisterUsage()
{
    std::string testName = "CodeGen: Register Usage";

    IRFunction func("reg_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue z("z", "int", 0);

    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &z, IROperand(x), IROperand(y)));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(z)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify that actual registers are used (not placeholder text)
    bool usesRegisters = (assembly.find("%rax") != std::string::npos ||
                          assembly.find("%rcx") != std::string::npos ||
                          assembly.find("%rdx") != std::string::npos ||
                          assembly.find("%rsi") != std::string::npos ||
                          assembly.find("%rdi") != std::string::npos);

    bool passed = usesRegisters;

    reportTest(testName, passed);
}

// ============================================================================
// Test 14: Code Generator - Return Value in RAX
// ============================================================================
void test_CodeGenReturnInRAX()
{
    std::string testName = "CodeGen: Return Value in RAX";

    IRFunction func("return_test", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue result("result", "int", 0);
    block->addInstruction(std::make_unique<MoveInst>(
        &result, IROperand("123", IROperand::OperandType::CONSTANT)));
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // By System V ABI, return value must be in RAX
    // We should see a move to %rax before return (unless already in RAX)
    bool hasRAX = assembly.find("%rax") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasRAX && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 15: Code Generator - Multiple Comparisons
// ============================================================================
void test_CodeGenMultipleComparisons()
{
    std::string testName = "CodeGen: Multiple Comparisons";

    IRFunction func("multi_cmp", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue c1("c", "int", 1);
    SSAValue c2("c", "int", 2);

    // c1 = a < b
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::LT, &c1, IROperand(a), IROperand(b)));

    // c2 = a == b
    block->addInstruction(std::make_unique<ComparisonInst>(
        IROpcode::EQ, &c2, IROperand(a), IROperand(b)));

    // return c2
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(c2)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Should have two comparison operations
    bool hasSetl = assembly.find("setl") != std::string::npos;
    bool hasSete = assembly.find("sete") != std::string::npos;

    bool passed = hasSetl && hasSete;

    reportTest(testName, passed);
}

// ============================================================================
// Test 16: Code Generator - Division
// ============================================================================
void test_CodeGenDivision()
{
    std::string testName = "CodeGen: Division (a / b)";

    IRFunction func("divide", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue result("result", "int", 0);

    // result = a / b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::DIV, &result, IROperand(a), IROperand(b)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify division instruction and RAX/RDX handling
    bool hasIdiv = assembly.find("idivq") != std::string::npos;
    bool hasCqto = assembly.find("cqto") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasIdiv && hasCqto && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Test 17: Code Generator - Modulo
// ============================================================================
void test_CodeGenModulo()
{
    std::string testName = "CodeGen: Modulo (a % b)";

    IRFunction func("modulo", "int");
    auto block = std::make_unique<IRBasicBlock>("entry");

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue result("result", "int", 0);

    // result = a % b
    block->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::MOD, &result, IROperand(a), IROperand(b)));

    // return result
    block->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(block));

    CodeGenerator codegen;
    std::string assembly = codegen.generateFunction(&func);

    // Verify modulo uses division instruction
    bool hasIdiv = assembly.find("idivq") != std::string::npos;
    bool hasCqto = assembly.find("cqto") != std::string::npos;
    bool hasReturn = assembly.find("ret") != std::string::npos;

    bool passed = hasIdiv && hasCqto && hasReturn;

    reportTest(testName, passed);
}

// ============================================================================
// Main - Run All Tests
// ============================================================================

int main()
{
    std::cout << "========================================\n";
    std::cout << "x86-64 Code Generation Tests\n";
    std::cout << "========================================\n\n";

    // Register Allocation Tests
    std::cout << "--- Linear Scan Register Allocator ---\n";
    test_LinearScanBuildIntervals();
    test_LinearScanSimpleAllocation();
    test_LinearScanMultipleValues();

    std::cout << "\n--- Code Generation Tests ---\n";
    test_CodeGenSimpleArithmetic();
    test_CodeGenSubtraction();
    test_CodeGenMultiplication();
    test_CodeGenDivision();
    test_CodeGenModulo();
    test_CodeGenComparison();
    test_CodeGenMove();
    test_CodeGenConstant();
    test_CodeGenMultipleInstructions();

    std::cout << "\n--- ABI Compliance Tests ---\n";
    test_CodeGenPrologueEpilogue();
    test_CodeGenATTSyntax();
    test_CodeGenRegisterUsage();
    test_CodeGenReturnInRAX();
    test_CodeGenMultipleComparisons();

    std::cout << "\n========================================\n";
    std::cout << "Test Results: " << passedTests << "/" << totalTests << " passed\n";
    std::cout << "========================================\n";

    return (passedTests == totalTests) ? 0 : 1;
}
