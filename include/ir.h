#ifndef IR_H
#define IR_H

#include <memory>
#include <string>
#include <variant>
#include <vector>

// ============================================================================
// SSA IR (Static Single Assignment Intermediate Representation)
// ============================================================================
// This IR uses SSA form where each variable is assigned exactly once.
// PHI nodes are used at control flow merge points to handle multiple
// definitions.

// Forward declarations
class IRInstruction;
class IRBasicBlock;
class IRFunction;
class TempVarGenerator;

// ============================================================================
// SSA Value - Represents a value in SSA form
// ============================================================================
class SSAValue
{
private:
  std::string name; // SSA variable name (e.g., "x_1", "temp_0")
  std::string type; // Type of the value (e.g., "int", "float", "int*")
  int version;      // SSA version number

public:
  SSAValue(const std::string &n, const std::string &t, int v = 0)
      : name(n), type(t), version(v) {}

  std::string getName() const { return name; }
  std::string getType() const { return type; }
  int getVersion() const { return version; }

  // Returns the full SSA name with version (e.g., "x_1")
  std::string getSSAName() const
  {
    return name + "_" + std::to_string(version);
  }
};

// ============================================================================
// IR Operand - Represents an operand in an instruction
// ============================================================================
class IROperand
{
public:
  enum class OperandType
  {
    SSA_VALUE, // SSA variable reference
    CONSTANT,  // Constant value (integer, float, string)
    LABEL      // Label reference (for jumps)
  };

private:
  OperandType opType;
  std::variant<SSAValue, std::string>
      value; // SSAValue or constant/label string

public:
  // Constructor for SSA value
  IROperand(const SSAValue &val) : opType(OperandType::SSA_VALUE), value(val) {}

  // Constructor for constant or label
  IROperand(const std::string &val, OperandType type)
      : opType(type), value(val) {}

  OperandType getType() const { return opType; }

  bool isSSAValue() const { return opType == OperandType::SSA_VALUE; }
  bool isConstant() const { return opType == OperandType::CONSTANT; }
  bool isLabel() const { return opType == OperandType::LABEL; }

  const SSAValue &getSSAValue() const { return std::get<SSAValue>(value); }

  const std::string &getConstant() const
  {
    return std::get<std::string>(value);
  }

  const std::string &getLabel() const { return std::get<std::string>(value); }

  std::string toString() const;
};

// ============================================================================
// IR Instruction Types (Opcodes)
// ============================================================================
enum class IROpcode
{
  // Arithmetic operations
  ADD, // result = op1 + op2
  SUB, // result = op1 - op2
  MUL, // result = op1 * op2
  DIV, // result = op1 / op2
  MOD, // result = op1 % op2

  // Comparison operations
  EQ, // result = op1 == op2
  NE, // result = op1 != op2
  LT, // result = op1 < op2
  GT, // result = op1 > op2
  LE, // result = op1 <= op2
  GE, // result = op1 >= op2

  // Control flow
  LABEL,         // Label for jump targets
  JUMP,          // Unconditional jump
  JUMP_IF_FALSE, // Conditional jump (if operand is false)

  // Memory operations
  LOAD,  // Load from memory address
  STORE, // Store to memory address

  // Function operations
  CALL,   // Function call
  RETURN, // Return from function
  PARAM,  // Function parameter

  // Assignment
  MOVE, // Simple assignment/copy

  // SSA-specific
  PHI // PHI node for SSA form (merge values from different paths)
};

// ============================================================================
// IR Instruction Base Class
// ============================================================================
class IRInstruction
{
protected:
  IROpcode opcode;
  SSAValue
      *result;                     // Result value (can be nullptr for instructions without results)
  std::vector<IROperand> operands; // Instruction operands

public:
  IRInstruction(IROpcode op, SSAValue *res = nullptr)
      : opcode(op), result(res) {}

  virtual ~IRInstruction() = default;

  IROpcode getOpcode() const { return opcode; }
  SSAValue *getResult() const { return result; }
  const std::vector<IROperand> &getOperands() const { return operands; }

  void addOperand(const IROperand &operand) { operands.push_back(operand); }

  virtual std::string toString() const = 0;
};

// ============================================================================
// Arithmetic Instructions
// ============================================================================
class ArithmeticInst : public IRInstruction
{
public:
  ArithmeticInst(IROpcode op, SSAValue *result, const IROperand &left,
                 const IROperand &right)
      : IRInstruction(op, result)
  {
    operands.push_back(left);
    operands.push_back(right);
  }

  ~ArithmeticInst() override;

  const IROperand &getLeft() const { return operands[0]; }
  const IROperand &getRight() const { return operands[1]; }

  std::string toString() const override;
};

// ============================================================================
// Comparison Instructions
// ============================================================================
class ComparisonInst : public IRInstruction
{
public:
  ComparisonInst(IROpcode op, SSAValue *result, const IROperand &left,
                 const IROperand &right)
      : IRInstruction(op, result)
  {
    operands.push_back(left);
    operands.push_back(right);
  }

  ~ComparisonInst() override;

  const IROperand &getLeft() const { return operands[0]; }
  const IROperand &getRight() const { return operands[1]; }

  std::string toString() const override;
};

// ============================================================================
// Control Flow Instructions
// ============================================================================
class LabelInst : public IRInstruction
{
private:
  std::string labelName;

public:
  LabelInst(const std::string &name)
      : IRInstruction(IROpcode::LABEL), labelName(name)
  {
    // Add label to operands so emitLabelInst can access it
    operands.push_back(IROperand(name, IROperand::OperandType::LABEL));
  }

  ~LabelInst() override;

  const std::string &getLabelName() const { return labelName; }

  std::string toString() const override;
};

class JumpInst : public IRInstruction
{
private:
  std::string targetLabel;

public:
  JumpInst(const std::string &target)
      : IRInstruction(IROpcode::JUMP), targetLabel(target)
  {
    operands.push_back(IROperand(target, IROperand::OperandType::LABEL));
  }

  ~JumpInst() override;

  const std::string &getTargetLabel() const { return targetLabel; }

  std::string toString() const override;
};

class JumpIfFalseInst : public IRInstruction
{
private:
  std::string targetLabel;

public:
  JumpIfFalseInst(const IROperand &condition, const std::string &target)
      : IRInstruction(IROpcode::JUMP_IF_FALSE), targetLabel(target)
  {
    operands.push_back(condition);
    operands.push_back(IROperand(target, IROperand::OperandType::LABEL));
  }

  ~JumpIfFalseInst() override;

  const IROperand &getCondition() const { return operands[0]; }
  const std::string &getTargetLabel() const { return targetLabel; }

  std::string toString() const override;
};

// ============================================================================
// Memory Instructions
// ============================================================================
class LoadInst : public IRInstruction
{
public:
  LoadInst(SSAValue *result, const IROperand &address)
      : IRInstruction(IROpcode::LOAD, result)
  {
    operands.push_back(address);
  }

  ~LoadInst() override;

  const IROperand &getAddress() const { return operands[0]; }

  std::string toString() const override;
};

class StoreInst : public IRInstruction
{
public:
  StoreInst(const IROperand &value, const IROperand &address)
      : IRInstruction(IROpcode::STORE)
  {
    operands.push_back(value);
    operands.push_back(address);
  }

  ~StoreInst() override;

  const IROperand &getValue() const { return operands[0]; }
  const IROperand &getAddress() const { return operands[1]; }

  std::string toString() const override;
};

// ============================================================================
// Function Instructions
// ============================================================================
class CallInst : public IRInstruction
{
private:
  std::string functionName;

public:
  CallInst(const std::string &funcName, SSAValue *result = nullptr)
      : IRInstruction(IROpcode::CALL, result), functionName(funcName) {}

  ~CallInst() override;

  const std::string &getFunctionName() const { return functionName; }

  void addArgument(const IROperand &arg) { operands.push_back(arg); }

  std::string toString() const override;
};

class ReturnInst : public IRInstruction
{
public:
  // Constructor for return with value
  ReturnInst(const IROperand &value) : IRInstruction(IROpcode::RETURN)
  {
    operands.push_back(value);
  }

  // Constructor for void return
  ReturnInst() : IRInstruction(IROpcode::RETURN) {}

  ~ReturnInst() override;

  bool hasReturnValue() const { return !operands.empty(); }
  const IROperand &getReturnValue() const { return operands[0]; }

  std::string toString() const override;
};

class ParamInst : public IRInstruction
{
private:
  std::string paramName;
  int paramIndex;

public:
  ParamInst(SSAValue *result, const std::string &name, int index)
      : IRInstruction(IROpcode::PARAM, result), paramName(name),
        paramIndex(index) {}

  ~ParamInst() override;

  const std::string &getParamName() const { return paramName; }
  int getParamIndex() const { return paramIndex; }

  std::string toString() const override;
};

// ============================================================================
// Assignment Instruction
// ============================================================================
class MoveInst : public IRInstruction
{
public:
  MoveInst(SSAValue *result, const IROperand &source)
      : IRInstruction(IROpcode::MOVE, result)
  {
    operands.push_back(source);
  }

  ~MoveInst() override;

  const IROperand &getSource() const { return operands[0]; }

  std::string toString() const override;
};

// ============================================================================
// PHI Instruction (SSA-specific)
// ============================================================================
// PHI nodes are used in SSA form to merge values from different control flow
// paths
class PhiInst : public IRInstruction
{
private:
  // Each PHI operand is paired with the label of the basic block it comes from
  std::vector<std::pair<IROperand, std::string>> incomingValues;

public:
  PhiInst(SSAValue *result) : IRInstruction(IROpcode::PHI, result) {}

  ~PhiInst() override;

  void addIncoming(const IROperand &value, const std::string &blockLabel)
  {
    incomingValues.push_back({value, blockLabel});
    operands.push_back(value);
  }

  const std::vector<std::pair<IROperand, std::string>> &
  getIncomingValues() const
  {
    return incomingValues;
  }

  std::string toString() const override;
};

// ============================================================================
// IR Basic Block
// ============================================================================
// A basic block is a sequence of instructions with a single entry and exit
// point
class IRBasicBlock
{
private:
  std::string label;
  std::vector<std::unique_ptr<IRInstruction>> instructions;
  std::vector<IRBasicBlock *>
      predecessors;                       // Blocks that can jump to this block
  std::vector<IRBasicBlock *> successors; // Blocks this block can jump to

public:
  IRBasicBlock(const std::string &lbl) : label(lbl) {}

  const std::string &getLabel() const { return label; }

  void addInstruction(std::unique_ptr<IRInstruction> inst)
  {
    instructions.push_back(std::move(inst));
  }

  const std::vector<std::unique_ptr<IRInstruction>> &getInstructions() const
  {
    return instructions;
  }

  void addPredecessor(IRBasicBlock *block) { predecessors.push_back(block); }

  void addSuccessor(IRBasicBlock *block) { successors.push_back(block); }

  const std::vector<IRBasicBlock *> &getPredecessors() const
  {
    return predecessors;
  }
  const std::vector<IRBasicBlock *> &getSuccessors() const
  {
    return successors;
  }

  std::string toString() const;
};

// ============================================================================
// IR Function
// ============================================================================
// Represents a function in IR form
class IRFunction
{
private:
  std::string name;
  std::string returnType;
  bool isExtern;
  std::vector<SSAValue> parameters;
  std::vector<std::unique_ptr<IRBasicBlock>> basicBlocks;

public:
  IRFunction(const std::string &funcName, const std::string &retType,
             bool external = false)
      : name(funcName), returnType(retType), isExtern(external) {}

  const std::string &getName() const { return name; }
  const std::string &getReturnType() const { return returnType; }
  bool getIsExtern() const { return isExtern; }

  void addParameter(const SSAValue &param) { parameters.push_back(param); }

  void addBasicBlock(std::unique_ptr<IRBasicBlock> block)
  {
    basicBlocks.push_back(std::move(block));
  }

  const std::vector<SSAValue> &getParameters() const { return parameters; }
  const std::vector<std::unique_ptr<IRBasicBlock>> &getBasicBlocks() const
  {
    return basicBlocks;
  }

  std::string toString() const;
};

// ============================================================================
// Temporary Variable Generator
// ============================================================================
// Generates unique temporary variable names for intermediate values
// Format: t0, t1, t2, t3, ...
class TempVarGenerator
{
private:
  int tempCount;      // Counter for temporary variables
  std::string prefix; // Prefix for temp variable names (default: "t")

public:
  // Constructor with optional custom prefix
  TempVarGenerator(const std::string &pfx = "t") : tempCount(0), prefix(pfx) {}

  // Generate a new unique temporary variable name
  // Returns: "t0", "t1", "t2", etc.
  std::string newTemp() { return prefix + std::to_string(tempCount++); }

  // Generate a new SSA temporary value with type
  // Returns: SSAValue with name "t0", "t1", etc.
  SSAValue newTempSSA(const std::string &type)
  {
    std::string name = newTemp();
    return SSAValue(name, type, 0);
  }

  // Get current temp count (useful for debugging/testing)
  int getTempCount() const { return tempCount; }

  // Reset the counter (useful when starting a new function)
  void reset() { tempCount = 0; }

  // Set custom prefix for temp variables
  void setPrefix(const std::string &pfx) { prefix = pfx; }

  // Get current prefix
  const std::string &getPrefix() const { return prefix; }
};

// ============================================================================
// Label Generator
// ============================================================================
// Generates unique label names for basic blocks
// Format: L0, L1, L2, L3, ...
class LabelGenerator
{
private:
  int labelCount;     // Counter for labels
  std::string prefix; // Prefix for label names (default: "L")

public:
  // Constructor with optional custom prefix
  LabelGenerator(const std::string &pfx = "L") : labelCount(0), prefix(pfx) {}

  // Generate a new unique label name
  // Returns: "L0", "L1", "L2", etc.
  std::string newLabel() { return prefix + std::to_string(labelCount++); }

  // Generate a named label with counter
  // Returns: "loop_0", "if_1", etc.
  std::string newLabel(const std::string &name)
  {
    return name + "_" + std::to_string(labelCount++);
  }

  // Get current label count (useful for debugging/testing)
  int getLabelCount() const { return labelCount; }

  // Reset the counter
  void reset() { labelCount = 0; }

  // Set custom prefix
  void setPrefix(const std::string &pfx) { prefix = pfx; }

  // Get current prefix
  const std::string &getPrefix() const { return prefix; }
};

#endif // IR_H
