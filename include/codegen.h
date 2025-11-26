#ifndef CODEGEN_H
#define CODEGEN_H

#include "ir.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <set>

// ============================================================================
// x86-64 Register Representation
// ============================================================================

enum class X86Register
{
    // General-purpose registers (64-bit)
    RAX, RBX, RCX, RDX,
    RSI, RDI, RBP, RSP,
    R8, R9, R10, R11,
    R12, R13, R14, R15,

    // No register assigned
    NONE
};

// ============================================================================
// Live Interval - Tracks lifetime of SSA values for register allocation
// ============================================================================

struct LiveInterval
{
    SSAValue* value;           // The SSA value this interval represents
    int start;                 // First instruction that defines/uses this value
    int end;                   // Last instruction that uses this value
    X86Register assignedReg;   // Allocated register (NONE if spilled)
    int spillSlot;             // Stack slot if spilled (-1 if not spilled)

    LiveInterval(SSAValue* val, int s, int e)
        : value(val), start(s), end(e),
          assignedReg(X86Register::NONE), spillSlot(-1) {}

    bool overlaps(const LiveInterval& other) const {
        return !(end < other.start || other.end < start);
    }

    bool operator<(const LiveInterval& other) const {
        return start < other.start;
    }
};

// ============================================================================
// Linear Scan Register Allocator
// ============================================================================
// Implements Linear Scan algorithm (Poletto & Sarkar 1999)
// - Simple, fast, produces good code
// - Allocates registers in a single pass over live intervals
// - Spills to stack when registers unavailable

class LinearScanAllocator
{
public:
    // All live intervals sorted by start point (public for CodeGenerator access)
    std::vector<LiveInterval> intervals;

private:
    // Available general-purpose registers for allocation
    // RSP and RBP are reserved for stack management
    std::vector<X86Register> availableRegisters;

    // Active intervals currently using registers
    std::vector<LiveInterval*> active;

    // Spill counter
    int nextSpillSlot;

    // Helper: Free register used by an interval
    void freeRegister(LiveInterval* interval);

    // Helper: Allocate a free register to an interval
    bool allocateFreeRegister(LiveInterval* interval);

    // Helper: Spill an interval to stack
    void spillInterval(LiveInterval* interval);

    // Helper: Expire old intervals that no longer need registers
    void expireOldIntervals(LiveInterval* current);

public:
    LinearScanAllocator();
    ~LinearScanAllocator() = default;

    // Build live intervals from IR function
    void buildLiveIntervals(IRFunction* function);

    // Run linear scan algorithm
    void allocate();

    // Get register assignment for an SSA value
    X86Register getRegister(const SSAValue* value) const;

    // Check if value is spilled
    bool isSpilled(const SSAValue* value) const;

    // Get spill slot for value
    int getSpillSlot(const SSAValue* value) const;

    // Get number of spill slots needed
    int getSpillSlotCount() const { return nextSpillSlot; }

    // Debug: Print allocation results
    void printAllocation() const;
};

// ============================================================================
// Peephole Optimizer - Assembly-level optimizations
// ============================================================================
// Performs local optimizations on generated assembly code
// - Removes redundant instructions (mov %rax, %rax)
// - Optimizes arithmetic with constants (add $0, mul by powers of 2)
// - Eliminates consecutive push/pop pairs
// - Simplifies instruction sequences

class PeepholeOptimizer
{
private:
    std::vector<std::string> instructions;  // Assembly instructions
    bool optimizationEnabled;

    // Optimization pattern detection
    bool isRedundantMove(const std::string& inst) const;
    bool isArithmeticWithZero(const std::string& inst) const;
    bool isMultiplyByPowerOfTwo(const std::string& inst, int& shiftAmount) const;
    bool isPushPopPair(size_t index) const;
    bool isRedundantComparison(size_t index) const;

    // Optimization transformations
    std::string optimizeMultiplyToShift(const std::string& inst, int shiftAmount) const;
    void removeInstruction(size_t index);
    void replaceInstruction(size_t index, const std::string& newInst);

public:
    PeepholeOptimizer();

    // Enable/disable optimizations
    void setEnabled(bool enable) { optimizationEnabled = enable; }
    bool isEnabled() const { return optimizationEnabled; }

    // Add instruction to optimizer
    void addInstruction(const std::string& inst);

    // Run optimization passes
    void optimize();

    // Get optimized assembly
    std::string getOptimizedCode() const;

    // Get optimization statistics
    int getInstructionCount() const { return instructions.size(); }

    // Reset optimizer
    void reset();
};

// ============================================================================
// x86-64 Code Generator
// ============================================================================
// Generates AT&T syntax x86-64 assembly from SSA IR
// - Implements System V AMD64 ABI calling convention
// - Uses linear scan register allocation
// - Produces GAS-compatible assembly

class CodeGenerator
{
private:
    std::ostringstream output;
    std::ostringstream dataSection;  // For .data section (string literals, globals)
    LinearScanAllocator allocator;
    PeepholeOptimizer peepholeOptimizer;  // Assembly-level optimizer

    // Current function being generated
    IRFunction* currentFunction;

    // Stack frame size (in bytes)
    int stackFrameSize;

    // Callee-saved registers used in function (for save/restore)
    std::set<X86Register> calleeSavedUsed;

    // Track if we need stack alignment adjustment
    bool needsStackAlignment;

    // Track external symbols (functions not defined in this module)
    std::set<std::string> externalSymbols;

    // Track defined functions (functions defined in this module)
    std::set<std::string> definedFunctions;

    // String literal management
    std::unordered_map<std::string, std::string> stringLiterals;  // content -> label
    int stringLiteralCounter;

    // Debug information support
    bool debugMode;                    // Enable debug symbol generation
    std::string sourceFileName;        // Current source file name
    int currentSourceLine;             // Current source line number
    std::set<std::string> emittedFiles; // Track which files have been declared

    // Optimization control
    bool peepholeOptimizationEnabled;  // Enable peephole optimizations

    // ========================================================================
    // Helper Methods - Register Names
    // ========================================================================

    std::string getRegisterName(X86Register reg, int size = 64) const;
    std::string getRegisterForValue(const SSAValue* value) const;
    std::string getOperandString(const IROperand& operand) const;

    // ========================================================================
    // Helper Methods - Code Emission
    // ========================================================================

    void emit(const std::string& instruction);
    void emitComment(const std::string& comment);
    void emitLabel(const std::string& label);

    // ========================================================================
    // Helper Methods - Stack Management
    // ========================================================================

    void emitPrologue();
    void emitEpilogue();
    void emitSpillLoad(const SSAValue* value, X86Register tempReg);
    void emitSpillStore(const SSAValue* value, X86Register tempReg);

    // ========================================================================
    // Helper Methods - ABI Compliance
    // ========================================================================

    void determineCalleeSavedRegisters();
    void saveCalleeSavedRegisters();
    void restoreCalleeSavedRegisters();
    void alignStackForCall(int numStackArgs);
    void cleanupStackAfterCall(int numStackArgs);
    bool isCalleeSaved(X86Register reg) const;

    // ========================================================================
    // Helper Methods - External Symbols & Data Section
    // ========================================================================

    void emitDataSection();
    void emitExternalDeclarations();
    void markExternalSymbol(const std::string& symbol);
    void markDefinedFunction(const std::string& funcName);

    // ========================================================================
    // Helper Methods - Debug Information
    // ========================================================================

    void emitFileDirective(const std::string& filename);
    void emitLocationDirective(int line, int column = 0);
    void emitFunctionDebugInfo(const std::string& funcName);
    void emitCFIDirectives();

    // ========================================================================
    // Instruction Emission
    // ========================================================================

    void emitArithmeticInst(const IRInstruction* inst);
    void emitDivisionInst(const IRInstruction* inst);
    void emitComparisonInst(const IRInstruction* inst);
    void emitMoveInst(const IRInstruction* inst);
    void emitJumpInst(const IRInstruction* inst);
    void emitBranchInst(const IRInstruction* inst);
    void emitReturnInst(const IRInstruction* inst);
    void emitLabelInst(const IRInstruction* inst);
    void emitCallInst(const IRInstruction* inst);
    void emitLoadInst(const IRInstruction* inst);
    void emitStoreInst(const IRInstruction* inst);
    void emitParamInst(const IRInstruction* inst);

public:
    CodeGenerator();
    ~CodeGenerator() = default;

    // ========================================================================
    // Main API
    // ========================================================================

    // Generate x86-64 assembly for a single function
    std::string generateFunction(IRFunction* function);

    // Generate x86-64 assembly for multiple functions
    std::string generateProgram(const std::vector<std::unique_ptr<IRFunction>>& functions);

    // Get generated assembly code
    std::string getAssembly() const { return output.str(); }

    // Reset generator state
    void reset();

    // ========================================================================
    // Public API for External Symbols & Data Section
    // ========================================================================

    // Add a string literal to the data section (returns label name)
    std::string addStringLiteral(const std::string& str);

    // ========================================================================
    // Public API for Debug Information
    // ========================================================================

    // Enable/disable debug symbol generation
    void setDebugMode(bool enable) { debugMode = enable; }
    bool isDebugMode() const { return debugMode; }

    // Set source file for debug info
    void setSourceFile(const std::string& filename) { sourceFileName = filename; }

    // ========================================================================
    // Public API for Peephole Optimization
    // ========================================================================

    // Enable/disable peephole optimizations
    void setPeepholeOptimization(bool enable) { peepholeOptimizationEnabled = enable; }
    bool isPeepholeOptimizationEnabled() const { return peepholeOptimizationEnabled; }
};

// ============================================================================
// Utility Functions
// ============================================================================

// Convert X86Register enum to string (AT&T syntax)
std::string registerToString(X86Register reg);

// Get register size suffix for instruction (b/w/l/q)
std::string getSizeSuffix(int bits);

#endif // CODEGEN_H
