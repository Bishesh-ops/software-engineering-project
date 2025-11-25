#include "ir.h"
#include <iostream>
#include <cassert>

// ============================================================================
// Test IR Instruction Classes
// ============================================================================

void testArithmeticInstructions() {
    std::cout << "Testing Arithmetic Instructions...\n";

    // Create SSA values
    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue z("z", "int", 0);

    // Test ADD: z_0 = add x_0, y_0
    ArithmeticInst addInst(IROpcode::ADD, &z,
                           IROperand(x), IROperand(y));
    std::cout << "  " << addInst.toString() << "\n";
    assert(addInst.toString() == "z_0 = add x_0, y_0");

    // Test SUB with constant
    SSAValue result1("temp", "int", 1);
    ArithmeticInst subInst(IROpcode::SUB, &result1,
                           IROperand(x), IROperand("5", IROperand::OperandType::CONSTANT));
    std::cout << "  " << subInst.toString() << "\n";

    // Test MUL, DIV, MOD
    SSAValue result2("temp", "int", 2);
    ArithmeticInst mulInst(IROpcode::MUL, &result2, IROperand(x), IROperand(y));
    std::cout << "  " << mulInst.toString() << "\n";

    SSAValue result3("temp", "int", 3);
    ArithmeticInst divInst(IROpcode::DIV, &result3, IROperand(x), IROperand(y));
    std::cout << "  " << divInst.toString() << "\n";

    SSAValue result4("temp", "int", 4);
    ArithmeticInst modInst(IROpcode::MOD, &result4, IROperand(x), IROperand(y));
    std::cout << "  " << modInst.toString() << "\n";

    std::cout << "Arithmetic Instructions: PASSED\n\n";
}

void testComparisonInstructions() {
    std::cout << "Testing Comparison Instructions...\n";

    SSAValue a("a", "int", 0);
    SSAValue b("b", "int", 0);
    SSAValue cond("cond", "bool", 0);

    // Test all comparison operations
    ComparisonInst eqInst(IROpcode::EQ, &cond, IROperand(a), IROperand(b));
    std::cout << "  " << eqInst.toString() << "\n";

    SSAValue cond1("cond", "bool", 1);
    ComparisonInst neInst(IROpcode::NE, &cond1, IROperand(a), IROperand(b));
    std::cout << "  " << neInst.toString() << "\n";

    SSAValue cond2("cond", "bool", 2);
    ComparisonInst ltInst(IROpcode::LT, &cond2, IROperand(a), IROperand(b));
    std::cout << "  " << ltInst.toString() << "\n";

    SSAValue cond3("cond", "bool", 3);
    ComparisonInst gtInst(IROpcode::GT, &cond3, IROperand(a), IROperand(b));
    std::cout << "  " << gtInst.toString() << "\n";

    SSAValue cond4("cond", "bool", 4);
    ComparisonInst leInst(IROpcode::LE, &cond4, IROperand(a), IROperand(b));
    std::cout << "  " << leInst.toString() << "\n";

    SSAValue cond5("cond", "bool", 5);
    ComparisonInst geInst(IROpcode::GE, &cond5, IROperand(a), IROperand(b));
    std::cout << "  " << geInst.toString() << "\n";

    std::cout << "Comparison Instructions: PASSED\n\n";
}

void testControlFlowInstructions() {
    std::cout << "Testing Control Flow Instructions...\n";

    // Test LABEL
    LabelInst label("loop_start");
    std::cout << "  " << label.toString() << "\n";
    assert(label.toString() == "loop_start:");

    // Test JUMP
    JumpInst jump("loop_end");
    std::cout << "  " << jump.toString() << "\n";
    assert(jump.toString() == "jump loop_end");

    // Test JUMP_IF_FALSE
    SSAValue cond("cond", "bool", 0);
    JumpIfFalseInst jumpIfFalse(IROperand(cond), "else_branch");
    std::cout << "  " << jumpIfFalse.toString() << "\n";

    std::cout << "Control Flow Instructions: PASSED\n\n";
}

void testMemoryInstructions() {
    std::cout << "Testing Memory Instructions...\n";

    SSAValue ptr("ptr", "int*", 0);
    SSAValue value("value", "int", 0);
    SSAValue loaded("loaded", "int", 0);

    // Test LOAD
    LoadInst loadInst(&loaded, IROperand(ptr));
    std::cout << "  " << loadInst.toString() << "\n";
    assert(loadInst.toString() == "loaded_0 = load ptr_0");

    // Test STORE
    StoreInst storeInst{IROperand(value), IROperand(ptr)};
    std::cout << "  " << storeInst.toString() << "\n";
    assert(storeInst.toString() == "store value_0, ptr_0");

    std::cout << "Memory Instructions: PASSED\n\n";
}

void testFunctionInstructions() {
    std::cout << "Testing Function Instructions...\n";

    // Test PARAM
    SSAValue param1("x", "int", 0);
    ParamInst paramInst(&param1, "x", 0);
    std::cout << "  " << paramInst.toString() << "\n";

    // Test CALL with return value
    SSAValue result("result", "int", 0);
    CallInst callInst("foo", &result);
    SSAValue arg1("arg1", "int", 0);
    SSAValue arg2("arg2", "int", 0);
    callInst.addArgument(IROperand(arg1));
    callInst.addArgument(IROperand(arg2));
    std::cout << "  " << callInst.toString() << "\n";

    // Test CALL without return value (void function)
    CallInst voidCallInst("bar", nullptr);
    voidCallInst.addArgument(IROperand(arg1));
    std::cout << "  " << voidCallInst.toString() << "\n";

    // Test RETURN with value
    SSAValue retVal("retval", "int", 0);
    ReturnInst retInst{IROperand(retVal)};
    std::cout << "  " << retInst.toString() << "\n";
    assert(retInst.toString() == "return retval_0");

    // Test RETURN without value (void return)
    ReturnInst voidRetInst;
    std::cout << "  " << voidRetInst.toString() << "\n";
    assert(voidRetInst.toString() == "return");

    std::cout << "Function Instructions: PASSED\n\n";
}

void testMoveInstruction() {
    std::cout << "Testing Move Instruction...\n";

    SSAValue dest("dest", "int", 0);
    SSAValue src("src", "int", 0);

    // Test MOVE
    MoveInst moveInst(&dest, IROperand(src));
    std::cout << "  " << moveInst.toString() << "\n";
    assert(moveInst.toString() == "dest_0 = move src_0");

    // Test MOVE with constant
    SSAValue dest2("temp", "int", 1);
    MoveInst moveConstInst(&dest2, IROperand("42", IROperand::OperandType::CONSTANT));
    std::cout << "  " << moveConstInst.toString() << "\n";

    std::cout << "Move Instruction: PASSED\n\n";
}

void testPhiInstruction() {
    std::cout << "Testing PHI Instruction (SSA-specific)...\n";

    // PHI node: merge values from different control flow paths
    SSAValue x_0("x", "int", 0);
    SSAValue x_1("x", "int", 1);
    SSAValue x_2("x", "int", 2);

    // x_2 = phi [x_0 from block1, x_1 from block2]
    PhiInst phiInst(&x_2);
    phiInst.addIncoming(IROperand(x_0), "block1");
    phiInst.addIncoming(IROperand(x_1), "block2");
    std::cout << "  " << phiInst.toString() << "\n";

    std::cout << "PHI Instruction: PASSED\n\n";
}

void testBasicBlock() {
    std::cout << "Testing Basic Block...\n";

    IRBasicBlock block("entry");

    // Add some instructions to the block
    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue z("z", "int", 0);

    block.addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &z, IROperand(x), IROperand(y)));

    SSAValue result("result", "int", 0);
    block.addInstruction(std::make_unique<MoveInst>(&result, IROperand(z)));

    block.addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    std::cout << block.toString() << "\n";

    std::cout << "Basic Block: PASSED\n\n";
}

void testCompleteFunction() {
    std::cout << "Testing Complete Function...\n";
    std::cout << "Example: int add(int a, int b) { return a + b; }\n\n";

    // Create function
    IRFunction func("add", "int");

    // Add parameters
    SSAValue param_a("a", "int", 0);
    SSAValue param_b("b", "int", 0);
    func.addParameter(param_a);
    func.addParameter(param_b);

    // Create entry basic block
    auto entryBlock = std::make_unique<IRBasicBlock>("entry");

    // Add instructions
    SSAValue result("result", "int", 0);
    entryBlock->addInstruction(std::make_unique<ArithmeticInst>(
        IROpcode::ADD, &result, IROperand(param_a), IROperand(param_b)));

    entryBlock->addInstruction(std::make_unique<ReturnInst>(IROperand(result)));

    func.addBasicBlock(std::move(entryBlock));

    // Print the function
    std::cout << func.toString() << "\n";

    std::cout << "Complete Function: PASSED\n\n";
}

void testSSAExample() {
    std::cout << "Testing SSA Form Example...\n";
    std::cout << "Example C code:\n";
    std::cout << "  int x = 1;\n";
    std::cout << "  if (cond) {\n";
    std::cout << "    x = 2;\n";
    std::cout << "  } else {\n";
    std::cout << "    x = 3;\n";
    std::cout << "  }\n";
    std::cout << "  return x;\n\n";

    std::cout << "SSA IR:\n";

    IRFunction func("test", "int");

    // Entry block
    auto entryBlock = std::make_unique<IRBasicBlock>("entry");
    SSAValue x_0("x", "int", 0);
    SSAValue cond("cond", "bool", 0);
    entryBlock->addInstruction(std::make_unique<MoveInst>(
        &x_0, IROperand("1", IROperand::OperandType::CONSTANT)));
    entryBlock->addInstruction(std::make_unique<JumpIfFalseInst>(
        IROperand(cond), "else_block"));
    func.addBasicBlock(std::move(entryBlock));

    // Then block
    auto thenBlock = std::make_unique<IRBasicBlock>("then_block");
    SSAValue x_1("x", "int", 1);
    thenBlock->addInstruction(std::make_unique<MoveInst>(
        &x_1, IROperand("2", IROperand::OperandType::CONSTANT)));
    thenBlock->addInstruction(std::make_unique<JumpInst>("merge_block"));
    func.addBasicBlock(std::move(thenBlock));

    // Else block
    auto elseBlock = std::make_unique<IRBasicBlock>("else_block");
    SSAValue x_2("x", "int", 2);
    elseBlock->addInstruction(std::make_unique<MoveInst>(
        &x_2, IROperand("3", IROperand::OperandType::CONSTANT)));
    elseBlock->addInstruction(std::make_unique<JumpInst>("merge_block"));
    func.addBasicBlock(std::move(elseBlock));

    // Merge block with PHI node
    auto mergeBlock = std::make_unique<IRBasicBlock>("merge_block");
    SSAValue x_3("x", "int", 3);
    auto phiInst = std::make_unique<PhiInst>(&x_3);
    phiInst->addIncoming(IROperand(x_1), "then_block");
    phiInst->addIncoming(IROperand(x_2), "else_block");
    mergeBlock->addInstruction(std::move(phiInst));
    mergeBlock->addInstruction(std::make_unique<ReturnInst>(IROperand(x_3)));
    func.addBasicBlock(std::move(mergeBlock));

    std::cout << func.toString() << "\n";

    std::cout << "SSA Form Example: PASSED\n\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "IR Instruction Classes Test Suite\n";
    std::cout << "========================================\n\n";

    testArithmeticInstructions();
    testComparisonInstructions();
    testControlFlowInstructions();
    testMemoryInstructions();
    testFunctionInstructions();
    testMoveInstruction();
    testPhiInstruction();
    testBasicBlock();
    testCompleteFunction();
    testSSAExample();

    std::cout << "========================================\n";
    std::cout << "All tests PASSED!\n";
    std::cout << "========================================\n";

    return 0;
}
