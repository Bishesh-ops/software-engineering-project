#ifndef IR_CODEGEN_H
#define IR_CODEGEN_H

#include "ast.h"
#include "ir.h"
#include <vector>
#include <memory>
#include <stack>

// ============================================================================
// IR Code Generator
// ============================================================================
// Converts AST expressions and statements into SSA-form IR instructions
// Uses the Visitor pattern to traverse the AST and generate corresponding IR
class IRCodeGenerator : public ASTVisitor
{
private:
    // Generators for unique names
    TempVarGenerator tempGen;
    LabelGenerator labelGen;

    // Current function being generated
    IRFunction* currentFunction;

    // Current basic block being populated
    IRBasicBlock* currentBlock;

    // Stack to hold expression results (SSA values or operands)
    std::stack<IROperand> exprStack;

    // Stack to hold result SSA values from expressions
    std::stack<SSAValue*> resultStack;

    // All generated instructions (for standalone expression generation)
    std::vector<std::unique_ptr<IRInstruction>> instructions;

    // Helper: Create a constant operand
    IROperand makeConstant(const std::string& value);

    // Helper: Create an SSA value operand
    IROperand makeSSAOperand(SSAValue* value);

    // Helper: Convert AST operator to IR opcode
    IROpcode operatorToOpcode(const std::string& op);

    // Helper: Add instruction to current block or instruction list
    void addInstruction(std::unique_ptr<IRInstruction> inst);

    // Helper: Get the default type for expressions (for now, use "int")
    std::string getDefaultType() const { return "int"; }

public:
    IRCodeGenerator();
    ~IRCodeGenerator() = default;

    // ========================================================================
    // Expression lowering methods
    // ========================================================================

    // Lower a literal expression to IR
    // Example: 42 -> MOVE t0, 42
    void visit(LiteralExpr& node) override;

    // Lower an identifier expression to IR
    // Example: variable "x" -> use "x" directly as operand
    void visit(IdentifierExpr& node) override;

    // Lower a binary expression to IR
    // Example: a + b -> t0 = ADD a, b
    // Example (nested): a + b * c -> t0 = MUL b, c; t1 = ADD a, t0
    void visit(BinaryExpr& node) override;

    // Lower a unary expression to IR
    void visit(UnaryExpr& node) override;

    // Lower a function call expression to IR
    void visit(CallExpr& node) override;

    // Lower an assignment expression to IR
    void visit(AssignmentExpr& node) override;

    // Lower an array access expression to IR
    void visit(ArrayAccessExpr& node) override;

    // Lower a member access expression to IR
    void visit(MemberAccessExpr& node) override;

    // Lower a type cast expression to IR
    void visit(TypeCastExpr& node) override;

    // ========================================================================
    // Statement lowering methods (stubs for now)
    // ========================================================================

    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(ForStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(CompoundStmt& node) override;
    void visit(ExpressionStmt& node) override;
    void visit(DeclStmt& node) override;

    // ========================================================================
    // Declaration lowering methods (stubs for now)
    // ========================================================================

    void visit(VarDecl& node) override;
    void visit(TypeDecl& node) override;
    void visit(StructDecl& node) override;
    void visit(FunctionDecl& node) override;
    void visit(ParameterDecl& node) override;

    // ========================================================================
    // Public API for expression lowering
    // ========================================================================

    // Generate IR for a standalone expression
    // Returns the list of generated instructions
    std::vector<std::unique_ptr<IRInstruction>> generateExpressionIR(Expression* expr);

    // Generate IR for a statement
    // Returns the list of generated instructions
    std::vector<std::unique_ptr<IRInstruction>> generateStatementIR(Statement* stmt);

    // Generate IR for a function declaration
    // Returns an IRFunction object
    std::unique_ptr<IRFunction> generateFunctionIR(FunctionDecl* funcDecl);

    // Get the result operand from the last expression evaluation
    IROperand getExpressionResult();

    // Get the result SSA value from the last expression evaluation
    SSAValue* getResultSSAValue();

    // Reset the code generator state
    void reset();

    // Get all generated instructions (for debugging/testing)
    const std::vector<std::unique_ptr<IRInstruction>>& getInstructions() const {
        return instructions;
    }
};

#endif // IR_CODEGEN_H
