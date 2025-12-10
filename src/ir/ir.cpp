#include "ir.h"
#include <sstream>

// ============================================================================
// IROperand Implementation
// ============================================================================
std::string IROperand::toString() const {
    switch (opType) {
        case OperandType::SSA_VALUE:
            return getSSAValue().getSSAName();
        case OperandType::CONSTANT:
            return getConstant();
        case OperandType::LABEL:
            return getLabel();
        default:
            return "<unknown>";
    }
}

// ============================================================================
// Helper function to get opcode string
// ============================================================================
static std::string opcodeToString(IROpcode opcode) {
    switch (opcode) {
        case IROpcode::ADD: return "add";
        case IROpcode::SUB: return "sub";
        case IROpcode::MUL: return "mul";
        case IROpcode::DIV: return "div";
        case IROpcode::MOD: return "mod";
        case IROpcode::EQ: return "eq";
        case IROpcode::NE: return "ne";
        case IROpcode::LT: return "lt";
        case IROpcode::GT: return "gt";
        case IROpcode::LE: return "le";
        case IROpcode::GE: return "ge";
        case IROpcode::LABEL: return "label";
        case IROpcode::JUMP: return "jump";
        case IROpcode::JUMP_IF_FALSE: return "jump_if_false";
        case IROpcode::LOAD: return "load";
        case IROpcode::STORE: return "store";
        case IROpcode::CALL: return "call";
        case IROpcode::RETURN: return "return";
        case IROpcode::PARAM: return "param";
        case IROpcode::MOVE: return "move";
        case IROpcode::PHI: return "phi";
        default: return "<unknown>";
    }
}

// ============================================================================
// Virtual Destructors for Derived Classes
// ============================================================================
ArithmeticInst::~ArithmeticInst() = default;
ComparisonInst::~ComparisonInst() = default;
LabelInst::~LabelInst() = default;
JumpInst::~JumpInst() = default;
JumpIfFalseInst::~JumpIfFalseInst() = default;
LoadInst::~LoadInst() = default;
StoreInst::~StoreInst() = default;
CallInst::~CallInst() = default;
ReturnInst::~ReturnInst() = default;
ParamInst::~ParamInst() = default;
MoveInst::~MoveInst() = default;
PhiInst::~PhiInst() = default;

// ============================================================================
// ArithmeticInst Implementation
// ============================================================================
std::string ArithmeticInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << opcodeToString(opcode) << " "
        << getLeft().toString() << ", "
        << getRight().toString();
    return oss.str();
}

// ============================================================================
// ComparisonInst Implementation
// ============================================================================
std::string ComparisonInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << opcodeToString(opcode) << " "
        << getLeft().toString() << ", "
        << getRight().toString();
    return oss.str();
}

// ============================================================================
// LabelInst Implementation
// ============================================================================
std::string LabelInst::toString() const {
    return labelName + ":";
}

// ============================================================================
// JumpInst Implementation
// ============================================================================
std::string JumpInst::toString() const {
    return "jump " + targetLabel;
}

// ============================================================================
// JumpIfFalseInst Implementation
// ============================================================================
std::string JumpIfFalseInst::toString() const {
    std::ostringstream oss;
    oss << "jump_if_false " << getCondition().toString() << ", " << targetLabel;
    return oss.str();
}

// ============================================================================
// LoadInst Implementation
// ============================================================================
std::string LoadInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << "load " << getAddress().toString();
    return oss.str();
}

// ============================================================================
// StoreInst Implementation
// ============================================================================
std::string StoreInst::toString() const {
    std::ostringstream oss;
    oss << "store " << getValue().toString() << ", " << getAddress().toString();
    return oss.str();
}

// ============================================================================
// CallInst Implementation
// ============================================================================
std::string CallInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << "call " << functionName << "(";
    for (size_t i = 0; i < operands.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << operands[i].toString();
    }
    oss << ")";
    return oss.str();
}

// ============================================================================
// ReturnInst Implementation
// ============================================================================
std::string ReturnInst::toString() const {
    std::ostringstream oss;
    oss << "return";
    if (hasReturnValue()) {
        oss << " " << getReturnValue().toString();
    }
    return oss.str();
}

// ============================================================================
// ParamInst Implementation
// ============================================================================
std::string ParamInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << "param " << paramName << " [" << paramIndex << "]";
    return oss.str();
}

// ============================================================================
// MoveInst Implementation
// ============================================================================
std::string MoveInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << "move " << getSource().toString();
    return oss.str();
}

// ============================================================================
// PhiInst Implementation
// ============================================================================
std::string PhiInst::toString() const {
    std::ostringstream oss;
    if (result) {
        oss << result->getSSAName() << " = ";
    }
    oss << "phi [";
    for (size_t i = 0; i < incomingValues.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << incomingValues[i].first.toString() << " from " << incomingValues[i].second;
    }
    oss << "]";
    return oss.str();
}

// ============================================================================
// IRBasicBlock Implementation
// ============================================================================
std::string IRBasicBlock::toString() const {
    std::ostringstream oss;
    oss << label << ":\n";
    for (const auto& inst : instructions) {
        oss << "  " << inst->toString() << "\n";
    }
    return oss.str();
}

// ============================================================================
// IRFunction Implementation
// ============================================================================
std::string IRFunction::toString() const {
    std::ostringstream oss;
    oss << "function " << returnType << " " << name << "(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << parameters[i].getType() << " " << parameters[i].getSSAName();
    }
    oss << ") {\n";

    for (const auto& block : basicBlocks) {
        oss << block->toString();
    }

    oss << "}\n";
    return oss.str();
}
