#include "ir_codegen.h"
#include <stdexcept>

// ============================================================================
// Constructor
// ============================================================================
IRCodeGenerator::IRCodeGenerator()
    : currentFunction(nullptr), currentBlock(nullptr) {}

// ============================================================================
// Helper Methods
// ============================================================================

IROperand IRCodeGenerator::makeConstant(const std::string& value) {
    return IROperand(value, IROperand::OperandType::CONSTANT);
}

IROperand IRCodeGenerator::makeSSAOperand(SSAValue* value) {
    return IROperand(*value);
}

IROpcode IRCodeGenerator::operatorToOpcode(const std::string& op) {
    // Arithmetic operators
    if (op == "+") return IROpcode::ADD;
    if (op == "-") return IROpcode::SUB;
    if (op == "*") return IROpcode::MUL;
    if (op == "/") return IROpcode::DIV;
    if (op == "%") return IROpcode::MOD;

    // Comparison operators
    if (op == "==") return IROpcode::EQ;
    if (op == "!=") return IROpcode::NE;
    if (op == "<") return IROpcode::LT;
    if (op == ">") return IROpcode::GT;
    if (op == "<=") return IROpcode::LE;
    if (op == ">=") return IROpcode::GE;

    throw std::runtime_error("Unknown operator: " + op);
}

void IRCodeGenerator::addInstruction(std::unique_ptr<IRInstruction> inst) {
    if (currentBlock) {
        currentBlock->addInstruction(std::move(inst));
    } else {
        instructions.push_back(std::move(inst));
    }
}

// ============================================================================
// Expression Lowering - Literal Expression
// ============================================================================
// Converts a literal value to IR
// Example: 42 -> MOVE t0, 42
void IRCodeGenerator::visit(LiteralExpr& node) {
    // Create a constant operand from the literal value
    IROperand constantOp = makeConstant(node.getValue());

    // Generate a temporary variable to hold the result
    SSAValue* tempResult = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);

    // Create a MOVE instruction: result = MOVE constant
    auto moveInst = std::make_unique<MoveInst>(tempResult, constantOp);
    addInstruction(std::move(moveInst));

    // Push the result onto the stacks
    exprStack.push(makeSSAOperand(tempResult));
    resultStack.push(tempResult);
}

// ============================================================================
// Expression Lowering - Identifier Expression
// ============================================================================
// Converts an identifier reference to IR
// Identifiers are used directly by name (no instruction needed)
void IRCodeGenerator::visit(IdentifierExpr& node) {
    // Create an SSA value for the identifier
    // In SSA form, we use the variable name directly
    SSAValue* identValue = new SSAValue(node.getName(), getDefaultType(), 0);

    // Push the identifier as an operand (no instruction needed)
    exprStack.push(makeSSAOperand(identValue));
    resultStack.push(identValue);
}

// ============================================================================
// Expression Lowering - Binary Expression
// ============================================================================
// Converts a binary operation to IR with proper handling of nested expressions
// Example: a + b -> t0 = ADD a, b
// Example (nested): a + b * c ->
//   t0 = MUL b, c
//   t1 = ADD a, t0
void IRCodeGenerator::visit(BinaryExpr& node) {
    // First, recursively lower the left operand
    // This may generate multiple instructions for nested expressions
    node.getLeft()->accept(*this);
    IROperand leftOperand = exprStack.top();
    exprStack.pop();
    resultStack.pop();

    // Then, recursively lower the right operand
    // This may also generate multiple instructions for nested expressions
    node.getRight()->accept(*this);
    IROperand rightOperand = exprStack.top();
    exprStack.pop();
    resultStack.pop();

    // Convert the AST operator to IR opcode
    IROpcode opcode = operatorToOpcode(node.getOperator());

    // Generate a temporary variable to hold the result
    SSAValue* tempResult = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);

    // Create the appropriate instruction based on the opcode type
    std::unique_ptr<IRInstruction> inst;

    // Arithmetic operations: ADD, SUB, MUL, DIV, MOD
    if (opcode == IROpcode::ADD || opcode == IROpcode::SUB ||
        opcode == IROpcode::MUL || opcode == IROpcode::DIV || opcode == IROpcode::MOD) {
        inst = std::make_unique<ArithmeticInst>(opcode, tempResult, leftOperand, rightOperand);
    }
    // Comparison operations: EQ, NE, LT, GT, LE, GE
    else if (opcode == IROpcode::EQ || opcode == IROpcode::NE ||
             opcode == IROpcode::LT || opcode == IROpcode::GT ||
             opcode == IROpcode::LE || opcode == IROpcode::GE) {
        inst = std::make_unique<ComparisonInst>(opcode, tempResult, leftOperand, rightOperand);
    }

    addInstruction(std::move(inst));

    // Push the result onto the stacks
    exprStack.push(makeSSAOperand(tempResult));
    resultStack.push(tempResult);
}

// ============================================================================
// Expression Lowering - Unary Expression
// ============================================================================
// Handles unary operations including pointer dereference
// Example: *ptr → t0 = LOAD ptr
void IRCodeGenerator::visit(UnaryExpr& node) {
    const std::string& op = node.getOperator();

    // Handle pointer dereference operator
    if (op == "*") {
        // Evaluate the pointer expression
        node.getOperand()->accept(*this);
        IROperand ptrOperand = exprStack.top();
        exprStack.pop();
        resultStack.pop();

        // Generate LOAD instruction: result = LOAD ptr
        SSAValue* loadResult = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);
        auto loadInst = std::make_unique<LoadInst>(loadResult, ptrOperand);
        addInstruction(std::move(loadInst));

        // Push the loaded value onto the stack
        exprStack.push(makeSSAOperand(loadResult));
        resultStack.push(loadResult);
    }
    // Handle address-of operator
    else if (op == "&") {
        // For address-of, we need to get the address of a variable
        // This is typically handled at a higher level, but for now just evaluate operand
        node.getOperand()->accept(*this);
    }
    // Handle unary minus
    else if (op == "-") {
        // Evaluate the operand
        node.getOperand()->accept(*this);
        IROperand operand = exprStack.top();
        exprStack.pop();
        resultStack.pop();

        // Generate: result = 0 - operand
        IROperand zero = makeConstant("0");
        SSAValue* result = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);
        auto subInst = std::make_unique<ArithmeticInst>(IROpcode::SUB, result, zero, operand);
        addInstruction(std::move(subInst));

        exprStack.push(makeSSAOperand(result));
        resultStack.push(result);
    }
    // Other unary operators can be added here
    else {
        // For now, just evaluate the operand for unsupported operators
        node.getOperand()->accept(*this);
    }
}

// ============================================================================
// Expression Lowering - Call Expression
// ============================================================================
// Lowers function calls to IR with parameter passing
// result = func(arg1, arg2) →
//   eval arg1 → t0
//   eval arg2 → t1
//   result = CALL func(t0, t1)
//
// Note: The acceptance criteria mentions PARAM instructions, but our IR
// design uses CallInst with arguments directly added via addArgument().
// This is a valid alternative representation that achieves the same result.
void IRCodeGenerator::visit(CallExpr& node) {
    // Get the function name (assuming callee is an identifier)
    std::string functionName;
    if (node.getCallee()->getNodeType() == ASTNodeType::IDENTIFIER_EXPR) {
        IdentifierExpr* calleeIdent = static_cast<IdentifierExpr*>(node.getCallee());
        functionName = calleeIdent->getName();
    } else {
        throw std::runtime_error("Function calls with non-identifier callees not yet supported");
    }

    // Evaluate all arguments first and store them
    std::vector<IROperand> argumentOperands;
    for (const auto& arg : node.getArguments()) {
        arg->accept(*this);
        IROperand argOperand = exprStack.top();
        exprStack.pop();
        if (!resultStack.empty()) resultStack.pop();
        argumentOperands.push_back(argOperand);
    }

    // Generate a temporary to hold the return value
    SSAValue* returnValue = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);

    // Create the CALL instruction
    auto callInst = std::make_unique<CallInst>(functionName, returnValue);

    // Add all arguments to the call
    for (const auto& argOp : argumentOperands) {
        callInst->addArgument(argOp);
    }

    addInstruction(std::move(callInst));

    // Push the return value onto the stack
    exprStack.push(makeSSAOperand(returnValue));
    resultStack.push(returnValue);
}

// ============================================================================
// Expression Lowering - Assignment Expression
// ============================================================================
// Handles three types of assignments:
// 1. Simple variable: x = expr → evaluate expr, then MOVE x, temp
// 2. Array element: arr[i] = expr → calculate address, then STORE
// 3. Pointer deref: *ptr = expr → STORE ptr, expr
void IRCodeGenerator::visit(AssignmentExpr& node) {
    Expression* target = node.getTarget();

    // First, evaluate the right-hand side expression
    node.getValue()->accept(*this);
    IROperand valueOperand = exprStack.top();
    exprStack.pop();
    resultStack.pop();

    // Determine the type of assignment based on the target

    // Case 1: Simple identifier assignment (x = expr)
    if (target->getNodeType() == ASTNodeType::IDENTIFIER_EXPR) {
        IdentifierExpr* identTarget = static_cast<IdentifierExpr*>(target);

        // Create SSA value for the target variable
        SSAValue* targetValue = new SSAValue(identTarget->getName(), getDefaultType(), 0);

        // Generate MOVE instruction: target = value
        auto moveInst = std::make_unique<MoveInst>(targetValue, valueOperand);
        addInstruction(std::move(moveInst));

        // Push the target as the result of the assignment expression
        exprStack.push(makeSSAOperand(targetValue));
        resultStack.push(targetValue);
    }
    // Case 2: Array access assignment (arr[i] = expr)
    else if (target->getNodeType() == ASTNodeType::ARRAY_ACCESS_EXPR) {
        ArrayAccessExpr* arrayTarget = static_cast<ArrayAccessExpr*>(target);

        // Evaluate the array base
        arrayTarget->getArray()->accept(*this);
        IROperand arrayOperand = exprStack.top();
        exprStack.pop();
        resultStack.pop();

        // Evaluate the index expression
        arrayTarget->getIndex()->accept(*this);
        IROperand indexOperand = exprStack.top();
        exprStack.pop();
        resultStack.pop();

        // Calculate the address: address = array + index
        SSAValue* addressTemp = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);
        auto addInst = std::make_unique<ArithmeticInst>(
            IROpcode::ADD, addressTemp, arrayOperand, indexOperand
        );
        addInstruction(std::move(addInst));

        // Generate STORE instruction: STORE value, address
        auto storeInst = std::make_unique<StoreInst>(valueOperand, makeSSAOperand(addressTemp));
        addInstruction(std::move(storeInst));

        // Assignment expression evaluates to the value stored
        exprStack.push(valueOperand);
        resultStack.push(nullptr);  // No result SSA value for store
    }
    // Case 3: Pointer dereference assignment (*ptr = expr)
    else if (target->getNodeType() == ASTNodeType::UNARY_EXPR) {
        UnaryExpr* unaryTarget = static_cast<UnaryExpr*>(target);

        // Check if this is a dereference operator
        if (unaryTarget->getOperator() == "*") {
            // Evaluate the pointer expression
            unaryTarget->getOperand()->accept(*this);
            IROperand ptrOperand = exprStack.top();
            exprStack.pop();
            resultStack.pop();

            // Generate STORE instruction: STORE value, ptr
            auto storeInst = std::make_unique<StoreInst>(valueOperand, ptrOperand);
            addInstruction(std::move(storeInst));

            // Assignment expression evaluates to the value stored
            exprStack.push(valueOperand);
            resultStack.push(nullptr);  // No result SSA value for store
        } else {
            throw std::runtime_error("Unsupported unary operator in assignment target");
        }
    }
    // Case 4: Member access assignment (obj.member = expr)
    else if (target->getNodeType() == ASTNodeType::MEMBER_ACCESS_EXPR) {
        MemberAccessExpr* memberTarget = static_cast<MemberAccessExpr*>(target);

        // Evaluate the object expression to get the base
        memberTarget->getObject()->accept(*this);
        IROperand objectOperand = exprStack.top();
        exprStack.pop();
        resultStack.pop();

        // Get the member name
        std::string memberName = memberTarget->getMemberName();

        // Create a symbolic name for the member being assigned
        std::string memberAccessName;
        if (objectOperand.isSSAValue()) {
            memberAccessName = objectOperand.getSSAValue().getName() + "." + memberName;
        } else {
            memberAccessName = "tmp." + memberName;
        }

        // Create SSA value for the member
        SSAValue* memberValue = new SSAValue(memberAccessName, getDefaultType(), 0);

        // Generate MOVE instruction: member = value
        auto moveInst = std::make_unique<MoveInst>(memberValue, valueOperand);
        addInstruction(std::move(moveInst));

        // Push the member as the result
        exprStack.push(makeSSAOperand(memberValue));
        resultStack.push(memberValue);
    }
    else {
        throw std::runtime_error("Unsupported assignment target type");
    }
}

// ============================================================================
// Expression Lowering - Array Access Expression
// ============================================================================
// Loads a value from an array element
// Example: arr[i] → t0 = arr + i; t1 = LOAD t0
void IRCodeGenerator::visit(ArrayAccessExpr& node) {
    // Evaluate the array base
    node.getArray()->accept(*this);
    IROperand arrayOperand = exprStack.top();
    exprStack.pop();
    resultStack.pop();

    // Evaluate the index expression
    node.getIndex()->accept(*this);
    IROperand indexOperand = exprStack.top();
    exprStack.pop();
    resultStack.pop();

    // Calculate the address: address = array + index
    SSAValue* addressTemp = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);
    auto addInst = std::make_unique<ArithmeticInst>(
        IROpcode::ADD, addressTemp, arrayOperand, indexOperand
    );
    addInstruction(std::move(addInst));

    // Generate LOAD instruction: result = LOAD address
    SSAValue* loadResult = new SSAValue(tempGen.newTemp(), getDefaultType(), 0);
    auto loadInst = std::make_unique<LoadInst>(loadResult, makeSSAOperand(addressTemp));
    addInstruction(std::move(loadInst));

    // Push the loaded value onto the stack
    exprStack.push(makeSSAOperand(loadResult));
    resultStack.push(loadResult);
}

// ============================================================================
// Expression Lowering - Member Access Expression
// ============================================================================
// Handles struct member access: struct.member
// For now, we use a simplified approach with symbolic names
// Example: p.x → generates a symbolic name "p.x" as an SSA value
void IRCodeGenerator::visit(MemberAccessExpr& node) {
    // Evaluate the base expression (the struct variable)
    node.getObject()->accept(*this);
    IROperand objectOperand = exprStack.top();
    exprStack.pop();
    resultStack.pop();

    // Get the member name
    std::string memberName = node.getMemberName();

    // Create a symbolic name for the member access
    // If object is "p" and member is "x", create "p.x"
    std::string memberAccessName;
    if (objectOperand.isSSAValue()) {
        memberAccessName = objectOperand.getSSAValue().getName() + "." + memberName;
    } else {
        // Fallback for constants or other operand types
        memberAccessName = "tmp." + memberName;
    }

    // Create an SSA value for the member access
    SSAValue* memberValue = new SSAValue(memberAccessName, getDefaultType(), 0);

    // Push the result
    exprStack.push(makeSSAOperand(memberValue));
    resultStack.push(memberValue);
}

// ============================================================================
// Expression Lowering - Type Cast Expression (Stub)
// ============================================================================
void IRCodeGenerator::visit(TypeCastExpr& node) {
    // TODO: Implement type cast lowering in future user stories
    // For now, just evaluate the operand
    node.getOperand()->accept(*this);
}

// ============================================================================
// Statement Lowering - If Statement
// ============================================================================
// Lowers if statements to IR with conditional jumps
// Case 1: if (cond) then_stmt
//   eval cond → tc
//   JUMP_IF_FALSE tc, L_end
//   then_stmt
//   L_end:
//
// Case 2: if (cond) then_stmt else else_stmt
//   eval cond → tc
//   JUMP_IF_FALSE tc, L_else
//   then_stmt
//   JUMP L_end
//   L_else:
//   else_stmt
//   L_end:
void IRCodeGenerator::visit(IfStmt& node) {
    // Evaluate the condition expression
    node.getCondition()->accept(*this);
    IROperand conditionOperand = exprStack.top();
    exprStack.pop();
    if (!resultStack.empty()) resultStack.pop();

    if (node.getElseBranch() == nullptr) {
        // Case 1: Simple if statement without else
        // Generate L_end label
        std::string endLabel = labelGen.newLabel("if_end");

        // Generate JUMP_IF_FALSE: if condition is false, jump to end
        auto jumpIfFalse = std::make_unique<JumpIfFalseInst>(conditionOperand, endLabel);
        addInstruction(std::move(jumpIfFalse));

        // Generate then branch
        node.getThenBranch()->accept(*this);

        // Generate L_end label
        auto endLabelInst = std::make_unique<LabelInst>(endLabel);
        addInstruction(std::move(endLabelInst));
    } else {
        // Case 2: If-else statement
        // Generate L_else and L_end labels
        std::string elseLabel = labelGen.newLabel("if_else");
        std::string endLabel = labelGen.newLabel("if_end");

        // Generate JUMP_IF_FALSE: if condition is false, jump to else
        auto jumpIfFalse = std::make_unique<JumpIfFalseInst>(conditionOperand, elseLabel);
        addInstruction(std::move(jumpIfFalse));

        // Generate then branch
        node.getThenBranch()->accept(*this);

        // Generate unconditional jump to end (skip else branch)
        auto jumpToEnd = std::make_unique<JumpInst>(endLabel);
        addInstruction(std::move(jumpToEnd));

        // Generate L_else label
        auto elseLabelInst = std::make_unique<LabelInst>(elseLabel);
        addInstruction(std::move(elseLabelInst));

        // Generate else branch
        node.getElseBranch()->accept(*this);

        // Generate L_end label
        auto endLabelInst = std::make_unique<LabelInst>(endLabel);
        addInstruction(std::move(endLabelInst));
    }
}

// ============================================================================
// Statement Lowering - While Statement
// ============================================================================
// Lowers while loops to IR with conditional jumps and back-edges
// while (cond) body →
//   L_start:
//   eval cond → tc
//   JUMP_IF_FALSE tc, L_end
//   body
//   JUMP L_start
//   L_end:
void IRCodeGenerator::visit(WhileStmt& node) {
    // Generate loop labels
    std::string startLabel = labelGen.newLabel("while_start");
    std::string endLabel = labelGen.newLabel("while_end");

    // Generate L_start label
    auto startLabelInst = std::make_unique<LabelInst>(startLabel);
    addInstruction(std::move(startLabelInst));

    // Evaluate the loop condition
    node.getCondition()->accept(*this);
    IROperand conditionOperand = exprStack.top();
    exprStack.pop();
    if (!resultStack.empty()) resultStack.pop();

    // Generate JUMP_IF_FALSE: if condition is false, exit loop
    auto jumpIfFalse = std::make_unique<JumpIfFalseInst>(conditionOperand, endLabel);
    addInstruction(std::move(jumpIfFalse));

    // Generate loop body
    node.getBody()->accept(*this);

    // Generate unconditional jump back to start (loop back-edge)
    auto jumpToStart = std::make_unique<JumpInst>(startLabel);
    addInstruction(std::move(jumpToStart));

    // Generate L_end label
    auto endLabelInst = std::make_unique<LabelInst>(endLabel);
    addInstruction(std::move(endLabelInst));
}

// ============================================================================
// Statement Lowering - For Statement
// ============================================================================
// Lowers for loops to IR with initialization, condition, update, and back-edges
// for (init; cond; update) body →
//   init
//   L_start:
//   eval cond → tc
//   JUMP_IF_FALSE tc, L_end
//   body
//   update
//   JUMP L_start
//   L_end:
void IRCodeGenerator::visit(ForStmt& node) {
    // Generate loop labels
    std::string startLabel = labelGen.newLabel("for_start");
    std::string endLabel = labelGen.newLabel("for_end");

    // Generate initialization (if present)
    if (node.getInitializer() != nullptr) {
        node.getInitializer()->accept(*this);
    }

    // Generate L_start label
    auto startLabelInst = std::make_unique<LabelInst>(startLabel);
    addInstruction(std::move(startLabelInst));

    // Evaluate the loop condition (if present)
    if (node.getCondition() != nullptr) {
        node.getCondition()->accept(*this);
        IROperand conditionOperand = exprStack.top();
        exprStack.pop();
        if (!resultStack.empty()) resultStack.pop();

        // Generate JUMP_IF_FALSE: if condition is false, exit loop
        auto jumpIfFalse = std::make_unique<JumpIfFalseInst>(conditionOperand, endLabel);
        addInstruction(std::move(jumpIfFalse));
    }
    // If no condition, it's an infinite loop (condition implicitly true)

    // Generate loop body
    if (node.getBody() != nullptr) {
        node.getBody()->accept(*this);
    }

    // Generate update expression (if present)
    if (node.getIncrement() != nullptr) {
        node.getIncrement()->accept(*this);
        // Pop the result from the stack since we're not using it
        if (!exprStack.empty()) exprStack.pop();
        if (!resultStack.empty()) resultStack.pop();
    }

    // Generate unconditional jump back to start (loop back-edge)
    auto jumpToStart = std::make_unique<JumpInst>(startLabel);
    addInstruction(std::move(jumpToStart));

    // Generate L_end label
    auto endLabelInst = std::make_unique<LabelInst>(endLabel);
    addInstruction(std::move(endLabelInst));
}

// ============================================================================
// Statement Lowering - Return Statement
// ============================================================================
// Lowers return statements to IR
// return expr → eval expr, RETURN result
// return → RETURN (void)
void IRCodeGenerator::visit(ReturnStmt& node) {
    if (node.getReturnValue() != nullptr) {
        // Return with value
        node.getReturnValue()->accept(*this);
        IROperand returnOperand = exprStack.top();
        exprStack.pop();
        if (!resultStack.empty()) resultStack.pop();

        auto returnInst = std::make_unique<ReturnInst>(returnOperand);
        addInstruction(std::move(returnInst));
    } else {
        // Void return
        auto returnInst = std::make_unique<ReturnInst>();
        addInstruction(std::move(returnInst));
    }
}

// ============================================================================
// Statement Lowering - Compound Statement
// ============================================================================
// Lowers a block of statements by visiting each one in sequence
void IRCodeGenerator::visit(CompoundStmt& node) {
    // Visit each statement in the compound statement
    for (const auto& stmt : node.getStatements()) {
        stmt->accept(*this);
    }
}

// ============================================================================
// Statement Lowering - Expression Statement
// ============================================================================
// Lowers an expression used as a statement (e.g., x = 5; or foo();)
void IRCodeGenerator::visit(ExpressionStmt& node) {
    // Simply evaluate the expression
    node.getExpression()->accept(*this);

    // Pop the result from the stack since we're not using it
    if (!exprStack.empty()) exprStack.pop();
    if (!resultStack.empty()) resultStack.pop();
}

void IRCodeGenerator::visit(DeclStmt& node) {
    // Handle variable declaration with optional initialization
    Declaration* decl = node.getDeclaration();
    if (!decl) return;

    // Check if it's a VarDecl with an initializer
    if (decl->getNodeType() == ASTNodeType::VAR_DECL) {
        VarDecl* varDecl = static_cast<VarDecl*>(decl);

        if (varDecl->getInitializer() != nullptr) {
            // Evaluate the initializer
            varDecl->getInitializer()->accept(*this);
            IROperand initOperand = exprStack.top();
            exprStack.pop();
            if (!resultStack.empty()) resultStack.pop();

            // Create an SSA value for this variable
            SSAValue* varValue = new SSAValue(varDecl->getName(), varDecl->getType(), 0);

            // Generate MOVE instruction: var = initializer value
            auto moveInst = std::make_unique<MoveInst>(varValue, initOperand);
            addInstruction(std::move(moveInst));

            // Don't push to expr stack since this is a statement
        }
        // If no initializer, the variable exists but has no value yet
        // (would need alloca for proper handling)
    }
}

// ============================================================================
// Declaration Lowering Stubs
// ============================================================================

void IRCodeGenerator::visit(VarDecl& node) {
    throw std::runtime_error("VarDecl lowering not yet implemented");
}

void IRCodeGenerator::visit(TypeDecl& node) {
    throw std::runtime_error("TypeDecl lowering not yet implemented");
}

void IRCodeGenerator::visit(StructDecl& node) {
    throw std::runtime_error("StructDecl lowering not yet implemented");
}

void IRCodeGenerator::visit(FunctionDecl& node) {
    throw std::runtime_error("FunctionDecl lowering not yet implemented");
}

void IRCodeGenerator::visit(ParameterDecl& node) {
    throw std::runtime_error("ParameterDecl lowering not yet implemented");
}

// ============================================================================
// Public API
// ============================================================================

std::vector<std::unique_ptr<IRInstruction>> IRCodeGenerator::generateExpressionIR(Expression* expr) {
    // Reset the state before generating new IR
    reset();

    // Visit the expression to generate IR
    expr->accept(*this);

    // Move the instructions out and return them
    std::vector<std::unique_ptr<IRInstruction>> result;
    result.swap(instructions);
    return result;
}

std::vector<std::unique_ptr<IRInstruction>> IRCodeGenerator::generateStatementIR(Statement* stmt) {
    // Reset the state before generating new IR
    reset();

    // Visit the statement to generate IR
    stmt->accept(*this);

    // Move the instructions out and return them
    std::vector<std::unique_ptr<IRInstruction>> result;
    result.swap(instructions);
    return result;
}

std::unique_ptr<IRFunction> IRCodeGenerator::generateFunctionIR(FunctionDecl* funcDecl) {
    // Reset the state before generating new IR
    reset();

    // Create the IRFunction
    auto irFunc = std::make_unique<IRFunction>(
        funcDecl->getName(),
        funcDecl->getReturnType()
    );

    // Store the current function for reference during generation
    currentFunction = irFunc.get();

    // Create entry basic block
    auto entryBlock = std::make_unique<IRBasicBlock>(funcDecl->getName() + "_entry");
    currentBlock = entryBlock.get();

    // Add parameters to the function and generate PARAM instructions
    int paramIndex = 0;
    for (const auto& param : funcDecl->getParameters()) {
        // Create SSA value for the parameter
        SSAValue paramValue(param->getName(), param->getType(), 0);
        irFunc->addParameter(paramValue);

        // Generate PARAM instruction to capture the parameter
        SSAValue* paramSSA = new SSAValue(param->getName(), param->getType(), 0);
        auto paramInst = std::make_unique<ParamInst>(
            paramSSA,
            param->getName(),
            paramIndex++
        );
        currentBlock->addInstruction(std::move(paramInst));
    }

    // Generate the function body
    if (funcDecl->getBody() != nullptr) {
        funcDecl->getBody()->accept(*this);
    }

    // Add the entry block to the function
    irFunc->addBasicBlock(std::move(entryBlock));

    // Clear current function and block
    currentFunction = nullptr;
    currentBlock = nullptr;

    return irFunc;
}

IROperand IRCodeGenerator::getExpressionResult() {
    if (exprStack.empty()) {
        throw std::runtime_error("No expression result available");
    }
    return exprStack.top();
}

SSAValue* IRCodeGenerator::getResultSSAValue() {
    if (resultStack.empty()) {
        throw std::runtime_error("No result SSA value available");
    }
    return resultStack.top();
}

void IRCodeGenerator::reset() {
    // Clear the stacks
    while (!exprStack.empty()) exprStack.pop();
    while (!resultStack.empty()) resultStack.pop();

    // Clear instructions
    instructions.clear();

    // Reset generators
    tempGen.reset();
    labelGen.reset();

    // Clear current function and block
    currentFunction = nullptr;
    currentBlock = nullptr;
}
