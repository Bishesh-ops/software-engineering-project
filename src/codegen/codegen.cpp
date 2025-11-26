#include "codegen.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cassert>

// ============================================================================
// Utility Functions
// ============================================================================

std::string registerToString(X86Register reg)
{
    switch (reg) {
        case X86Register::RAX: return "rax";
        case X86Register::RBX: return "rbx";
        case X86Register::RCX: return "rcx";
        case X86Register::RDX: return "rdx";
        case X86Register::RSI: return "rsi";
        case X86Register::RDI: return "rdi";
        case X86Register::RBP: return "rbp";
        case X86Register::RSP: return "rsp";
        case X86Register::R8:  return "r8";
        case X86Register::R9:  return "r9";
        case X86Register::R10: return "r10";
        case X86Register::R11: return "r11";
        case X86Register::R12: return "r12";
        case X86Register::R13: return "r13";
        case X86Register::R14: return "r14";
        case X86Register::R15: return "r15";
        case X86Register::NONE: return "<none>";
        default: return "<unknown>";
    }
}

std::string getSizeSuffix(int bits)
{
    switch (bits) {
        case 8:  return "b";  // byte
        case 16: return "w";  // word
        case 32: return "l";  // long
        case 64: return "q";  // quad
        default: return "q";  // default to 64-bit
    }
}

// ============================================================================
// Linear Scan Register Allocator - Implementation
// ============================================================================

LinearScanAllocator::LinearScanAllocator()
    : nextSpillSlot(0)
{
    // Initialize available registers
    // System V AMD64 ABI:
    // - RAX, RCX, RDX, RSI, RDI, R8-R11 are caller-saved (volatile)
    // - RBX, R12-R15 are callee-saved (non-volatile)
    // - RBP, RSP are reserved for stack management
    //
    // For simplicity, we use caller-saved registers for allocation
    // (A production compiler would also use callee-saved with proper save/restore)

    availableRegisters = {
        X86Register::RAX,
        X86Register::RCX,
        X86Register::RDX,
        X86Register::RSI,
        X86Register::RDI,
        X86Register::R8,
        X86Register::R9,
        X86Register::R10,
        X86Register::R11
    };
}

void LinearScanAllocator::buildLiveIntervals(IRFunction* function)
{
    intervals.clear();
    nextSpillSlot = 0;

    // Map SSA values to their live intervals
    std::unordered_map<const SSAValue*, int> valueToInterval;

    int instructionIndex = 0;

    // First pass: Create intervals for all SSA values
    for (const auto& block : function->getBasicBlocks()) {
        for (const auto& inst : block->getInstructions()) {
            // If instruction defines a result, start a new interval
            if (inst->getResult() != nullptr) {
                SSAValue* result = inst->getResult();

                // Check if interval already exists
                auto it = valueToInterval.find(result);
                if (it == valueToInterval.end()) {
                    // Create new interval
                    intervals.emplace_back(result, instructionIndex, instructionIndex);
                    valueToInterval[result] = intervals.size() - 1;
                } else {
                    // Extend existing interval
                    intervals[it->second].end = instructionIndex;
                }
            }

            // For all operands used, extend their intervals
            for (const auto& operand : inst->getOperands()) {
                if (operand.isSSAValue()) {
                    const SSAValue& value = operand.getSSAValue();
                    auto it = valueToInterval.find(&value);
                    if (it != valueToInterval.end()) {
                        // Extend interval to this use
                        intervals[it->second].end = instructionIndex;
                    }
                }
            }

            instructionIndex++;
        }
    }

    // Sort intervals by start point (required for linear scan)
    std::sort(intervals.begin(), intervals.end());
}

void LinearScanAllocator::expireOldIntervals(LiveInterval* current)
{
    // Remove intervals from active list that no longer overlap with current
    auto it = active.begin();
    while (it != active.end()) {
        LiveInterval* interval = *it;

        // If interval ends before current starts, it's expired
        if (interval->end < current->start) {
            // Free the register
            freeRegister(interval);
            it = active.erase(it);
        } else {
            ++it;
        }
    }
}

void LinearScanAllocator::freeRegister(LiveInterval* interval)
{
    if (interval->assignedReg != X86Register::NONE) {
        // Add register back to available pool
        availableRegisters.push_back(interval->assignedReg);
    }
}

bool LinearScanAllocator::allocateFreeRegister(LiveInterval* interval)
{
    if (availableRegisters.empty()) {
        return false;
    }

    // Allocate first available register
    X86Register reg = availableRegisters.back();
    availableRegisters.pop_back();

    interval->assignedReg = reg;
    return true;
}

void LinearScanAllocator::spillInterval(LiveInterval* interval)
{
    // Assign a stack slot for spilling
    interval->spillSlot = nextSpillSlot++;
    interval->assignedReg = X86Register::NONE;
}

void LinearScanAllocator::allocate()
{
    active.clear();

    for (auto& interval : intervals) {
        // Expire old intervals that are no longer live
        expireOldIntervals(&interval);

        // Try to allocate a free register
        if (allocateFreeRegister(&interval)) {
            // Success! Add to active list
            active.push_back(&interval);
        } else {
            // No free registers, must spill
            // Strategy: Spill the interval that ends last (farthest use)

            // Find interval in active with latest end
            auto spillCandidate = std::max_element(active.begin(), active.end(),
                [](LiveInterval* a, LiveInterval* b) {
                    return a->end < b->end;
                });

            if (spillCandidate != active.end() && (*spillCandidate)->end > interval.end) {
                // Spill the candidate and use its register for current interval
                X86Register reg = (*spillCandidate)->assignedReg;
                spillInterval(*spillCandidate);

                interval.assignedReg = reg;

                // Replace spilled interval with current in active list
                *spillCandidate = &interval;
            } else {
                // Spill current interval
                spillInterval(&interval);
            }
        }
    }
}

X86Register LinearScanAllocator::getRegister(const SSAValue* value) const
{
    for (const auto& interval : intervals) {
        if (interval.value == value) {
            return interval.assignedReg;
        }
    }
    return X86Register::NONE;
}

bool LinearScanAllocator::isSpilled(const SSAValue* value) const
{
    for (const auto& interval : intervals) {
        if (interval.value == value) {
            return interval.spillSlot != -1;
        }
    }
    return false;
}

int LinearScanAllocator::getSpillSlot(const SSAValue* value) const
{
    for (const auto& interval : intervals) {
        if (interval.value == value) {
            return interval.spillSlot;
        }
    }
    return -1;
}

void LinearScanAllocator::printAllocation() const
{
    std::cout << "Register Allocation:\n";
    std::cout << "====================\n";

    for (const auto& interval : intervals) {
        std::cout << std::setw(20) << interval.value->getSSAName() << " -> ";

        if (interval.assignedReg != X86Register::NONE) {
            std::cout << "%" << registerToString(interval.assignedReg);
        } else {
            std::cout << "SPILL[" << interval.spillSlot << "]";
        }

        std::cout << "  (live: " << interval.start << "-" << interval.end << ")\n";
    }
}

// ============================================================================
// Peephole Optimizer - Implementation
// ============================================================================

PeepholeOptimizer::PeepholeOptimizer()
    : optimizationEnabled(true)
{
}

void PeepholeOptimizer::reset()
{
    instructions.clear();
}

void PeepholeOptimizer::addInstruction(const std::string& inst)
{
    instructions.push_back(inst);
}

bool PeepholeOptimizer::isRedundantMove(const std::string& inst) const
{
    // Pattern: movq %rax, %rax (moving register to itself)
    // Instructions may have leading whitespace
    std::string trimmed = inst;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));

    if (trimmed.find("movq ") != 0 && trimmed.find("movl ") != 0 &&
        trimmed.find("movw ") != 0 && trimmed.find("movb ") != 0) {
        return false;
    }

    // Extract source and destination
    size_t commaPos = trimmed.find(',');
    if (commaPos == std::string::npos) return false;

    size_t firstSpace = trimmed.find(' ');
    std::string src = trimmed.substr(firstSpace + 1, commaPos - firstSpace - 1);
    std::string dst = trimmed.substr(commaPos + 1);  // Skip ","

    // Trim whitespace
    src.erase(0, src.find_first_not_of(" \t"));
    src.erase(src.find_last_not_of(" \t\n\r") + 1);
    dst.erase(0, dst.find_first_not_of(" \t"));
    dst.erase(dst.find_last_not_of(" \t\n\r") + 1);

    return src == dst;
}

bool PeepholeOptimizer::isArithmeticWithZero(const std::string& inst) const
{
    // Pattern: addq $0, %rax or subq $0, %rax
    std::string trimmed = inst;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));

    if (trimmed.find("addq $0,") == 0 || trimmed.find("subq $0,") == 0 ||
        trimmed.find("addl $0,") == 0 || trimmed.find("subl $0,") == 0) {
        return true;
    }
    return false;
}

bool PeepholeOptimizer::isMultiplyByPowerOfTwo(const std::string& inst, int& shiftAmount) const
{
    // Pattern: imulq $N, %reg where N is a power of 2
    std::string trimmed = inst;
    trimmed.erase(0, trimmed.find_first_not_of(" \t"));

    if (trimmed.find("imulq $") != 0 && trimmed.find("imull $") != 0) {
        return false;
    }

    // Extract the constant
    size_t dollarPos = trimmed.find('$');
    size_t commaPos = trimmed.find(',');
    if (dollarPos == std::string::npos || commaPos == std::string::npos) {
        return false;
    }

    std::string constStr = trimmed.substr(dollarPos + 1, commaPos - dollarPos - 1);
    try {
        int value = std::stoi(constStr);

        // Check if power of 2 (only one bit set)
        if (value > 0 && (value & (value - 1)) == 0) {
            // Calculate shift amount
            shiftAmount = 0;
            while ((1 << shiftAmount) != value) {
                shiftAmount++;
            }
            return true;
        }
    } catch (...) {
        return false;
    }

    return false;
}

bool PeepholeOptimizer::isPushPopPair(size_t index) const
{
    // Pattern: pushq %rax followed by popq %rax
    if (index + 1 >= instructions.size()) return false;

    std::string inst1 = instructions[index];
    std::string inst2 = instructions[index + 1];

    // Trim leading whitespace
    inst1.erase(0, inst1.find_first_not_of(" \t"));
    inst2.erase(0, inst2.find_first_not_of(" \t"));

    if (inst1.find("pushq ") != 0 || inst2.find("popq ") != 0) {
        return false;
    }

    // Extract register from both
    std::string reg1 = inst1.substr(6);  // Skip "pushq "
    std::string reg2 = inst2.substr(5);  // Skip "popq "

    // Trim whitespace
    reg1.erase(0, reg1.find_first_not_of(" \t"));
    reg1.erase(reg1.find_last_not_of(" \t\n\r") + 1);
    reg2.erase(0, reg2.find_first_not_of(" \t"));
    reg2.erase(reg2.find_last_not_of(" \t\n\r") + 1);

    return reg1 == reg2;
}

bool PeepholeOptimizer::isRedundantComparison(size_t index) const
{
    // Pattern: cmpq followed immediately by another cmpq (second one overwrites flags)
    if (index + 1 >= instructions.size()) return false;

    std::string inst1 = instructions[index];
    std::string inst2 = instructions[index + 1];

    // Trim leading whitespace
    inst1.erase(0, inst1.find_first_not_of(" \t"));
    inst2.erase(0, inst2.find_first_not_of(" \t"));

    // Both must be compare instructions
    if ((inst1.find("cmpq ") != 0 && inst1.find("cmpl ") != 0) ||
        (inst2.find("cmpq ") != 0 && inst2.find("cmpl ") != 0)) {
        return false;
    }

    // Check if there's no conditional jump or setcc between them
    // If the next instruction is also a cmp, the first is redundant
    return true;
}

std::string PeepholeOptimizer::optimizeMultiplyToShift(const std::string& inst, int shiftAmount) const
{
    // Convert: imulq $8, %rax -> shlq $3, %rax
    // Preserve leading whitespace
    size_t firstNonSpace = inst.find_first_not_of(" \t");
    std::string leadingSpace = inst.substr(0, firstNonSpace);

    std::string trimmed = inst.substr(firstNonSpace);
    size_t commaPos = trimmed.find(',');
    std::string destReg = trimmed.substr(commaPos);  // Includes ", %reg"

    std::string prefix = (trimmed.find("imulq") == 0) ? "shlq" : "shll";
    return leadingSpace + prefix + " $" + std::to_string(shiftAmount) + destReg;
}

void PeepholeOptimizer::removeInstruction(size_t index)
{
    if (index < instructions.size()) {
        instructions.erase(instructions.begin() + index);
    }
}

void PeepholeOptimizer::replaceInstruction(size_t index, const std::string& newInst)
{
    if (index < instructions.size()) {
        instructions[index] = newInst;
    }
}

void PeepholeOptimizer::optimize()
{
    if (!optimizationEnabled) return;

    bool changed = true;
    int passes = 0;
    const int MAX_PASSES = 5;  // Prevent infinite loops

    while (changed && passes < MAX_PASSES) {
        changed = false;
        passes++;

        // Pass 1: Remove redundant moves
        for (size_t i = 0; i < instructions.size(); ) {
            if (isRedundantMove(instructions[i])) {
                removeInstruction(i);
                changed = true;
                // Don't increment i, check the same position again
            } else {
                i++;
            }
        }

        // Pass 2: Remove arithmetic with zero
        for (size_t i = 0; i < instructions.size(); ) {
            if (isArithmeticWithZero(instructions[i])) {
                removeInstruction(i);
                changed = true;
            } else {
                i++;
            }
        }

        // Pass 3: Convert multiply by power of 2 to shift
        for (size_t i = 0; i < instructions.size(); i++) {
            int shiftAmount;
            if (isMultiplyByPowerOfTwo(instructions[i], shiftAmount)) {
                std::string newInst = optimizeMultiplyToShift(instructions[i], shiftAmount);
                replaceInstruction(i, newInst);
                changed = true;
            }
        }

        // Pass 4: Remove push/pop pairs
        for (size_t i = 0; i < instructions.size(); ) {
            if (isPushPopPair(i)) {
                removeInstruction(i);  // Remove push
                removeInstruction(i);  // Remove pop (now at same index)
                changed = true;
            } else {
                i++;
            }
        }

        // Pass 5: Remove redundant comparisons
        for (size_t i = 0; i + 1 < instructions.size(); ) {
            if (isRedundantComparison(i)) {
                removeInstruction(i);  // Keep the second comparison
                changed = true;
            } else {
                i++;
            }
        }
    }
}

std::string PeepholeOptimizer::getOptimizedCode() const
{
    std::ostringstream result;
    for (const auto& inst : instructions) {
        result << inst;
        // Instructions should already have newlines
        if (!inst.empty() && inst.back() != '\n') {
            result << "\n";
        }
    }
    return result.str();
}

// ============================================================================
// Code Generator - Implementation
// ============================================================================

CodeGenerator::CodeGenerator()
    : currentFunction(nullptr), stackFrameSize(0), needsStackAlignment(false),
      stringLiteralCounter(0), debugMode(false), currentSourceLine(0),
      peepholeOptimizationEnabled(true)
{
}

void CodeGenerator::reset()
{
    output.str("");
    output.clear();
    dataSection.str("");
    dataSection.clear();
    currentFunction = nullptr;
    stackFrameSize = 0;
    calleeSavedUsed.clear();
    needsStackAlignment = false;
    externalSymbols.clear();
    definedFunctions.clear();
    stringLiterals.clear();
    stringLiteralCounter = 0;
    currentSourceLine = 0;
    emittedFiles.clear();
    peepholeOptimizer.reset();
}

std::string CodeGenerator::getRegisterName(X86Register reg, int size) const
{
    // Convert register to appropriate size variant
    // For AT&T syntax: %rax (64-bit), %eax (32-bit), %ax (16-bit), %al (8-bit)

    std::string base = registerToString(reg);

    if (size == 64) {
        return "%" + base;
    } else if (size == 32) {
        // Convert r?? to e??
        if (base[0] == 'r' && base.length() > 1 && base[1] != 's' && base[1] != 'b') {
            return "%e" + base.substr(1);
        }
        return "%" + base;  // r8-r15 stay the same
    } else if (size == 16) {
        if (base[0] == 'r' && base.length() > 1) {
            return "%" + base.substr(1);
        }
        return "%" + base;
    } else {
        // 8-bit: al, bl, cl, dl, etc.
        if (base[0] == 'r' && base.length() == 3) {
            return "%" + std::string(1, base[1]) + "l";
        }
        return "%" + base;
    }
}

std::string CodeGenerator::getRegisterForValue(const SSAValue* value) const
{
    X86Register reg = allocator.getRegister(value);

    if (reg == X86Register::NONE) {
        // Value is spilled, shouldn't call this directly
        return "<SPILLED>";
    }

    return getRegisterName(reg, 64);
}

std::string CodeGenerator::getOperandString(const IROperand& operand) const
{
    if (operand.isConstant()) {
        // Immediate value in AT&T syntax: $123
        return "$" + operand.getConstant();
    } else if (operand.isSSAValue()) {
        const SSAValue& value = operand.getSSAValue();

        // Check if spilled
        if (allocator.isSpilled(&value)) {
            // Return stack location: -8(%rbp) for first spill slot
            int slot = allocator.getSpillSlot(&value);
            int offset = -(slot + 1) * 8;  // Each slot is 8 bytes
            return std::to_string(offset) + "(%rbp)";
        } else {
            return getRegisterForValue(&value);
        }
    }

    return "<unknown>";
}

void CodeGenerator::emit(const std::string& instruction)
{
    // When peephole optimization is enabled, collect instructions for optimization
    // Comments, labels, and directives go directly to output
    if (peepholeOptimizationEnabled &&
        instruction.find('#') == std::string::npos &&  // Not a comment
        instruction.find(':') == std::string::npos &&  // Not a label
        instruction.find('.') != 0) {                   // Not a directive
        peepholeOptimizer.addInstruction("    " + instruction + "\n");
    } else {
        output << "    " << instruction << "\n";
    }
}

void CodeGenerator::emitComment(const std::string& comment)
{
    output << "    # " << comment << "\n";
}

void CodeGenerator::emitLabel(const std::string& label)
{
    output << label << ":\n";
}

// ============================================================================
// ABI Compliance Helper Methods
// ============================================================================

bool CodeGenerator::isCalleeSaved(X86Register reg) const
{
    // System V AMD64 ABI callee-saved (non-volatile) registers:
    // RBX, R12, R13, R14, R15, RBP
    return (reg == X86Register::RBX ||
            reg == X86Register::R12 ||
            reg == X86Register::R13 ||
            reg == X86Register::R14 ||
            reg == X86Register::R15 ||
            reg == X86Register::RBP);
}

void CodeGenerator::determineCalleeSavedRegisters()
{
    // Scan all allocated registers to see which callee-saved ones are used
    calleeSavedUsed.clear();

    // Check all live intervals to see what registers are allocated
    for (const auto& interval : allocator.intervals) {
        X86Register reg = interval.assignedReg;
        if (reg != X86Register::NONE && isCalleeSaved(reg)) {
            calleeSavedUsed.insert(reg);
        }
    }
}

void CodeGenerator::saveCalleeSavedRegisters()
{
    if (calleeSavedUsed.empty()) return;

    emitComment("Save callee-saved registers");
    for (X86Register reg : calleeSavedUsed) {
        if (reg != X86Register::RBP) {  // RBP already saved in prologue
            emit("pushq " + getRegisterName(reg, 64));
        }
    }
}

void CodeGenerator::restoreCalleeSavedRegisters()
{
    if (calleeSavedUsed.empty()) return;

    emitComment("Restore callee-saved registers");
    // Restore in reverse order
    std::vector<X86Register> regs(calleeSavedUsed.begin(), calleeSavedUsed.end());
    for (auto it = regs.rbegin(); it != regs.rend(); ++it) {
        if (*it != X86Register::RBP) {  // RBP restored in epilogue
            emit("popq " + getRegisterName(*it, 64));
        }
    }
}

void CodeGenerator::alignStackForCall(int numStackArgs)
{
    // System V AMD64 ABI requires 16-byte stack alignment before call
    // When we enter a function, RSP is misaligned by 8 (return address)
    // After push RBP, it's aligned to 16
    // We need to ensure it stays aligned before call

    // Calculate current stack position
    // After prologue: RSP = RBP - stackFrameSize
    // Each callee-saved register adds 8 bytes
    int calleeSavedBytes = 0;
    for (X86Register reg : calleeSavedUsed) {
        if (reg != X86Register::RBP) {
            calleeSavedBytes += 8;
        }
    }

    // Stack arguments will be pushed (numStackArgs * 8 bytes)
    int stackArgsBytes = numStackArgs * 8;

    // Total offset from RBP
    int totalOffset = stackFrameSize + calleeSavedBytes + stackArgsBytes;

    // After call instruction pushes return address (8 bytes),
    // we need (totalOffset + 8) to be aligned to 16
    int misalignment = (totalOffset + 8) % 16;

    if (misalignment != 0) {
        // Need to adjust stack
        int adjustment = 16 - misalignment;
        emit("subq $" + std::to_string(adjustment) + ", %rsp");
        needsStackAlignment = true;
        stackFrameSize += adjustment;  // Track for cleanup
    }
}

void CodeGenerator::cleanupStackAfterCall(int numStackArgs)
{
    // Remove stack arguments
    if (numStackArgs > 0) {
        emit("addq $" + std::to_string(numStackArgs * 8) + ", %rsp");
    }

    // Remove alignment adjustment if any
    if (needsStackAlignment) {
        // Alignment is already cleaned up in epilogue
        needsStackAlignment = false;
    }
}

void CodeGenerator::emitPrologue()
{
    emitComment("Function prologue - System V AMD64 ABI");

    // Emit CFI directives for debugging
    emitCFIDirectives();

    emit("pushq %rbp");

    if (debugMode) {
        // CFI: Indicate that RBP was pushed
        output << "    .cfi_def_cfa_offset 16\n";
        output << "    .cfi_offset %rbp, -16\n";
    }

    emit("movq %rsp, %rbp");

    if (debugMode) {
        // CFI: CFA (Canonical Frame Address) is now at RBP
        output << "    .cfi_def_cfa_register %rbp\n";
    }

    // Determine which callee-saved registers are used
    determineCalleeSavedRegisters();

    // Save callee-saved registers
    saveCalleeSavedRegisters();

    // Reserve stack space for spills
    int spillSlots = allocator.getSpillSlotCount();
    if (spillSlots > 0) {
        stackFrameSize = spillSlots * 8;
        // Align to 16 bytes (System V ABI requirement)
        if (stackFrameSize % 16 != 0) {
            stackFrameSize = ((stackFrameSize / 16) + 1) * 16;
        }
        emit("subq $" + std::to_string(stackFrameSize) + ", %rsp");
    }
    output << "\n";
}

void CodeGenerator::emitEpilogue()
{
    emitComment("Function epilogue - System V AMD64 ABI");

    // Restore stack pointer (deallocate local variables and spills)
    emit("movq %rbp, %rsp");

    // Restore callee-saved registers (in reverse order of save)
    restoreCalleeSavedRegisters();

    // Restore base pointer
    emit("popq %rbp");

    // Return to caller
    emit("ret");

    // End CFI directives
    if (debugMode) {
        output << "    .cfi_endproc\n";
    }
}

void CodeGenerator::emitSpillLoad(const SSAValue* value, X86Register tempReg)
{
    int slot = allocator.getSpillSlot(value);
    int offset = -(slot + 1) * 8;
    std::string tempRegStr = getRegisterName(tempReg, 64);
    emit("movq " + std::to_string(offset) + "(%rbp), " + tempRegStr);
}

void CodeGenerator::emitSpillStore(const SSAValue* value, X86Register tempReg)
{
    int slot = allocator.getSpillSlot(value);
    int offset = -(slot + 1) * 8;
    std::string tempRegStr = getRegisterName(tempReg, 64);
    emit("movq " + tempRegStr + ", " + std::to_string(offset) + "(%rbp)");
}

void CodeGenerator::emitArithmeticInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() != 2) return;

    SSAValue* result = inst->getResult();
    if (!result) return;

    std::string op;
    IROpcode opcode = inst->getOpcode();

    switch (opcode) {
        case IROpcode::ADD: op = "addq"; break;
        case IROpcode::SUB: op = "subq"; break;
        case IROpcode::MUL: op = "imulq"; break;
        case IROpcode::DIV:
        case IROpcode::MOD:
            // Division and modulo require special handling with RAX/RDX
            emitDivisionInst(inst);
            return;
        default:
            return;
    }

    std::string dest = getRegisterForValue(result);
    std::string src1 = getOperandString(operands[0]);
    std::string src2 = getOperandString(operands[1]);

    emitComment(result->getSSAName() + " = " +
                operands[0].toString() + " " +
                inst->toString().substr(0, 3) + " " +
                operands[1].toString());

    // x86-64 AT&T syntax: op source, dest
    // For binary ops: dest = dest op source

    // Load first operand into destination
    if (src1 != dest) {
        emit("movq " + src1 + ", " + dest);
    }

    // Apply operation
    emit(op + " " + src2 + ", " + dest);
}

void CodeGenerator::emitDivisionInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() != 2) return;

    SSAValue* result = inst->getResult();
    if (!result) return;

    IROpcode opcode = inst->getOpcode();
    std::string src1 = getOperandString(operands[0]);
    std::string src2 = getOperandString(operands[1]);
    std::string dest = getRegisterForValue(result);

    emitComment(result->getSSAName() + " = " +
                operands[0].toString() + (opcode == IROpcode::DIV ? " / " : " % ") +
                operands[1].toString());

    // x86-64 division requires:
    // - Dividend in RAX (and RDX for 128-bit)
    // - Divisor in register or memory
    // - idivq: quotient in RAX, remainder in RDX

    // Save RAX and RDX if they're being used
    emit("pushq %rax");
    emit("pushq %rdx");

    // Load dividend into RAX
    if (src1 != "%rax") {
        emit("movq " + src1 + ", %rax");
    }

    // Sign-extend RAX into RDX:RAX (for signed division)
    emit("cqto");

    // Perform division
    // If divisor is immediate, need to load into register first
    if (src2[0] == '$') {
        emit("movq " + src2 + ", %r11");
        emit("idivq %r11");
    } else {
        emit("idivq " + src2);
    }

    // Move result to destination
    if (opcode == IROpcode::DIV) {
        // Quotient is in RAX
        if (dest != "%rax") {
            emit("movq %rax, " + dest);
        }
    } else {
        // Remainder is in RDX
        if (dest != "%rdx") {
            emit("movq %rdx, " + dest);
        }
    }

    // Restore RAX and RDX
    emit("popq %rdx");
    emit("popq %rax");
}

void CodeGenerator::emitComparisonInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() != 2) return;

    SSAValue* result = inst->getResult();
    if (!result) return;

    std::string setcc;
    IROpcode opcode = inst->getOpcode();

    switch (opcode) {
        case IROpcode::EQ: setcc = "sete"; break;
        case IROpcode::NE: setcc = "setne"; break;
        case IROpcode::LT: setcc = "setl"; break;
        case IROpcode::GT: setcc = "setg"; break;
        case IROpcode::LE: setcc = "setle"; break;
        case IROpcode::GE: setcc = "setge"; break;
        default: return;
    }

    std::string dest = getRegisterForValue(result);
    std::string src1 = getOperandString(operands[0]);
    std::string src2 = getOperandString(operands[1]);

    emitComment(result->getSSAName() + " = " +
                operands[0].toString() + " cmp " +
                operands[1].toString());

    // Compare instruction
    emit("cmpq " + src2 + ", " + src1);

    // Set result (8-bit) based on comparison
    // Use lower 8-bit register variant
    std::string dest8bit = dest;
    // Convert %rax -> %al, %rbx -> %bl, etc.
    if (dest[1] == 'r') {
        dest8bit = "%" + dest.substr(2, dest.length() - 3) + "l";
    }

    emit(setcc + " " + dest8bit);

    // Zero-extend to 64-bit
    emit("movzbq " + dest8bit + ", " + dest);
}

void CodeGenerator::emitMoveInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() != 1) return;

    SSAValue* result = inst->getResult();
    if (!result) return;

    std::string dest = getRegisterForValue(result);
    std::string src = getOperandString(operands[0]);

    emitComment(result->getSSAName() + " = " + operands[0].toString());

    // Don't emit move if source and dest are the same
    if (src != dest) {
        emit("movq " + src + ", " + dest);
    }
}

void CodeGenerator::emitJumpInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() != 1) return;

    std::string target = operands[0].getConstant();
    emitComment("Unconditional jump to " + target);
    emit("jmp " + target);
}

void CodeGenerator::emitBranchInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() != 3) return;

    std::string condition = getOperandString(operands[0]);
    std::string trueLabel = operands[1].getConstant();
    std::string falseLabel = operands[2].getConstant();

    emitComment("Conditional branch");
    emit("cmpq $0, " + condition);
    emit("jne " + trueLabel);
    emit("jmp " + falseLabel);
}

void CodeGenerator::emitReturnInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();

    if (!operands.empty()) {
        // Return value convention: result in RAX
        std::string src = getOperandString(operands[0]);
        emitComment("Return " + operands[0].toString());

        if (src != "%rax") {
            emit("movq " + src + ", %rax");
        }
    } else {
        emitComment("Return void");
    }

    // Emit epilogue and return
    emitEpilogue();
}

void CodeGenerator::emitLabelInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.empty()) return;

    std::string labelName = operands[0].getConstant();
    output << "\n";
    emitLabel(labelName);
}

// ============================================================================
// External Symbols & Data Section Helper Methods
// ============================================================================

void CodeGenerator::markExternalSymbol(const std::string& symbol)
{
    // Only mark as external if it's not a function we're defining
    if (definedFunctions.find(symbol) == definedFunctions.end()) {
        externalSymbols.insert(symbol);
    }
}

void CodeGenerator::markDefinedFunction(const std::string& funcName)
{
    definedFunctions.insert(funcName);
    // Remove from external symbols if it was marked there
    externalSymbols.erase(funcName);
}

std::string CodeGenerator::addStringLiteral(const std::string& str)
{
    // Check if this string already exists
    auto it = stringLiterals.find(str);
    if (it != stringLiterals.end()) {
        return it->second;
    }

    // Create new label for this string
    std::string label = ".STR" + std::to_string(stringLiteralCounter++);
    stringLiterals[str] = label;

    // Add to data section
    dataSection << label << ":\n";
    dataSection << "    .asciz \"";

    // Escape special characters
    for (char c : str) {
        switch (c) {
            case '\n': dataSection << "\\n"; break;
            case '\t': dataSection << "\\t"; break;
            case '\r': dataSection << "\\r"; break;
            case '\\': dataSection << "\\\\"; break;
            case '\"': dataSection << "\\\""; break;
            default: dataSection << c; break;
        }
    }
    dataSection << "\"\n";

    return label;
}

void CodeGenerator::emitExternalDeclarations()
{
    if (externalSymbols.empty()) {
        return;
    }

    output << "# External function declarations\n";
    for (const auto& symbol : externalSymbols) {
        output << ".extern " << symbol << "\n";
    }
    output << "\n";
}

void CodeGenerator::emitDataSection()
{
    std::string dataStr = dataSection.str();
    if (dataStr.empty()) {
        return;
    }

    output << "# Data section for string literals and global data\n";
    output << ".data\n";
    output << dataStr;
    output << "\n";
}

// ============================================================================
// Debug Information Helper Methods
// ============================================================================

void CodeGenerator::emitFileDirective(const std::string& filename)
{
    if (!debugMode) return;

    // Only emit .file directive once per file
    if (emittedFiles.find(filename) != emittedFiles.end()) {
        return;
    }

    emittedFiles.insert(filename);
    output << ".file 1 \"" << filename << "\"\n";
}

void CodeGenerator::emitLocationDirective(int line, int column)
{
    if (!debugMode) return;
    if (line == currentSourceLine) return;  // Don't emit duplicate .loc for same line

    currentSourceLine = line;

    // .loc file_number line [column]
    // file_number is 1 (from .file directive)
    if (column > 0) {
        output << "    .loc 1 " << line << " " << column << "\n";
    } else {
        output << "    .loc 1 " << line << "\n";
    }
}

void CodeGenerator::emitFunctionDebugInfo(const std::string& funcName)
{
    if (!debugMode) return;

    // Emit function type information for debugging
    output << "    .type " << funcName << ", @function\n";
}

void CodeGenerator::emitCFIDirectives()
{
    if (!debugMode) return;

    // CFI (Call Frame Information) directives help debuggers unwind stack
    // These are essential for proper backtraces in gdb/lldb

    // .cfi_startproc marks the beginning of a function
    output << "    .cfi_startproc\n";
}

void CodeGenerator::emitCallInst(const IRInstruction* inst)
{
    // Get function name from CallInst
    const CallInst* callInst = dynamic_cast<const CallInst*>(inst);
    if (!callInst) return;

    std::string funcName = callInst->getFunctionName();

    // Mark this function as external if it's not defined in this module
    markExternalSymbol(funcName);

    emitComment("Call function: " + funcName + " (System V AMD64 ABI)");

    // System V AMD64 ABI: Integer/pointer arguments in RDI, RSI, RDX, RCX, R8, R9
    // Floating-point arguments in XMM0-XMM7 (not yet implemented)
    std::vector<std::string> paramRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    const auto& operands = inst->getOperands();

    // Count how many arguments will go on stack
    int stackArgs = 0;
    if (operands.size() > 6) {  // First 6 args in registers
        stackArgs = operands.size() - 6;
    }

    // Ensure stack is 16-byte aligned before call
    alignStackForCall(stackArgs);

    // Pass arguments
    // Note: Stack arguments are pushed in reverse order (right-to-left)
    std::vector<std::string> stackArgValues;

    for (size_t i = 0; i < operands.size(); i++) {
        std::string argValue = getOperandString(operands[i]);

        if (i < paramRegs.size()) {
            // Use register for parameter
            std::string reg = paramRegs[i];
            if (argValue != reg) {
                emit("movq " + argValue + ", " + reg);
            }
        } else {
            // Collect stack arguments (will push in reverse)
            stackArgValues.push_back(argValue);
        }
    }

    // Push stack arguments in reverse order
    for (auto it = stackArgValues.rbegin(); it != stackArgValues.rend(); ++it) {
        emit("pushq " + *it);
    }

    // Make the call
    emit("call " + funcName);

    // Clean up stack arguments and alignment
    cleanupStackAfterCall(stackArgs);

    // Result is in RAX (integer/pointer) or XMM0 (floating-point)
    if (inst->getResult()) {
        std::string dest = getRegisterForValue(inst->getResult());
        if (dest != "%rax") {
            emit("movq %rax, " + dest);
        }
    }
}

void CodeGenerator::emitLoadInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.empty()) return;

    SSAValue* result = inst->getResult();
    if (!result) return;

    std::string address = getOperandString(operands[0]);
    std::string dest = getRegisterForValue(result);

    emitComment("Load from memory: " + result->getSSAName() + " = *(" + operands[0].toString() + ")");

    // Load from address (assuming address is in a register or memory location)
    if (address[0] == '%') {
        // Address is in a register, dereference it
        emit("movq (" + address + "), " + dest);
    } else {
        // Address is a memory location or constant
        emit("movq " + address + ", %r11");
        emit("movq (%r11), " + dest);
    }
}

void CodeGenerator::emitStoreInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.size() < 2) return;

    std::string value = getOperandString(operands[0]);
    std::string address = getOperandString(operands[1]);

    emitComment("Store to memory: *(" + operands[1].toString() + ") = " + operands[0].toString());

    // Store value to address
    if (address[0] == '%') {
        // Address is in a register, dereference it
        if (value[0] == '$') {
            // Immediate value, need to use a temp register
            emit("movq " + value + ", %r11");
            emit("movq %r11, (" + address + ")");
        } else {
            emit("movq " + value + ", (" + address + ")");
        }
    } else {
        // Address is a memory location
        emit("movq " + address + ", %r11");
        if (value[0] == '$') {
            emit("movq " + value + ", %r10");
            emit("movq %r10, (%r11)");
        } else {
            emit("movq " + value + ", (%r11)");
        }
    }
}

void CodeGenerator::emitParamInst(const IRInstruction* inst)
{
    // PARAM instructions are typically handled during function prologue
    // They map parameters from calling convention registers to local variables
    const auto& operands = inst->getOperands();
    if (operands.empty()) return;

    SSAValue* result = inst->getResult();
    if (!result) return;

    emitComment("Parameter: " + result->getSSAName());

    // Get parameter index (from operand)
    int paramIndex = std::stoi(operands[0].getConstant());

    std::vector<std::string> paramRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    std::string dest = getRegisterForValue(result);

    if (paramIndex < (int)paramRegs.size()) {
        // Parameter is in a register
        std::string srcReg = paramRegs[paramIndex];
        if (dest != srcReg) {
            emit("movq " + srcReg + ", " + dest);
        }
    } else {
        // Parameter is on stack (above return address and saved RBP)
        int stackOffset = 16 + (paramIndex - 6) * 8;  // 8 for ret addr + 8 for saved RBP
        emit("movq " + std::to_string(stackOffset) + "(%rbp), " + dest);
    }
}

std::string CodeGenerator::generateFunction(IRFunction* function)
{
    currentFunction = function;

    // Mark this function as defined in this module
    markDefinedFunction(function->getName());

    // Reset peephole optimizer for this function
    if (peepholeOptimizationEnabled) {
        peepholeOptimizer.reset();
        peepholeOptimizer.setEnabled(true);
    }

    // Step 1: Build live intervals
    allocator.buildLiveIntervals(function);

    // Step 2: Allocate registers
    allocator.allocate();

    // Step 3: Generate assembly
    output << "\n";
    emitComment("Function: " + function->getName());

    // Emit debug information for function
    emitFunctionDebugInfo(function->getName());

    output << ".globl " << function->getName() << "\n";
    emitLabel(function->getName());

    // Emit prologue
    emitPrologue();

    // Emit instructions for each basic block
    for (const auto& block : function->getBasicBlocks()) {
        // Emit block label (if not entry)
        if (block->getLabel() != "entry") {
            output << "\n";
            emitLabel(block->getLabel());
        }

        // Emit each instruction
        for (const auto& inst : block->getInstructions()) {
            IROpcode opcode = inst->getOpcode();

            switch (opcode) {
                case IROpcode::ADD:
                case IROpcode::SUB:
                case IROpcode::MUL:
                case IROpcode::DIV:
                case IROpcode::MOD:
                    emitArithmeticInst(inst.get());
                    break;

                case IROpcode::EQ:
                case IROpcode::NE:
                case IROpcode::LT:
                case IROpcode::GT:
                case IROpcode::LE:
                case IROpcode::GE:
                    emitComparisonInst(inst.get());
                    break;

                case IROpcode::MOVE:
                    emitMoveInst(inst.get());
                    break;

                case IROpcode::JUMP:
                    emitJumpInst(inst.get());
                    break;

                case IROpcode::JUMP_IF_FALSE:
                    emitBranchInst(inst.get());
                    break;

                case IROpcode::RETURN:
                    emitReturnInst(inst.get());
                    break;

                case IROpcode::LABEL:
                    emitLabelInst(inst.get());
                    break;

                case IROpcode::CALL:
                    emitCallInst(inst.get());
                    break;

                case IROpcode::LOAD:
                    emitLoadInst(inst.get());
                    break;

                case IROpcode::STORE:
                    emitStoreInst(inst.get());
                    break;

                case IROpcode::PARAM:
                    emitParamInst(inst.get());
                    break;

                case IROpcode::PHI:
                    // PHI nodes are handled during SSA construction, not code generation
                    emitComment("PHI node (handled in SSA construction)");
                    break;

                default:
                    emitComment("Unhandled opcode");
                    break;
            }
        }
    }

    // Step 4: Apply peephole optimizations if enabled
    if (peepholeOptimizationEnabled) {
        peepholeOptimizer.optimize();
        output << peepholeOptimizer.getOptimizedCode();
    }

    return output.str();
}

std::string CodeGenerator::generateProgram(const std::vector<std::unique_ptr<IRFunction>>& functions)
{
    reset();

    // Emit assembly header with platform info
    output << "# Generated x86-64 assembly (AT&T syntax)\n";
    output << "# Target: System V AMD64 ABI\n";
    output << "# Platform: macOS/Linux compatible\n";
    output << "# Generated by C Compiler - Code Generation Phase\n";
    output << "# Supports external library integration (printf, malloc, etc.)\n";
    if (debugMode) {
        output << "# Debug symbols enabled for gdb/lldb debugging\n";
    }
    output << "\n";

    // Platform-specific directives
#ifdef __APPLE__
    // macOS uses different section names
    output << "# macOS Mach-O format\n";
#else
    // Linux ELF format
    output << "# Linux ELF format\n";
#endif
    output << "\n";

    // Emit file directive for debug information
    if (debugMode && !sourceFileName.empty()) {
        emitFileDirective(sourceFileName);
    }

    // Generate code for each function (this populates externalSymbols and dataSection)
    std::ostringstream tempOutput;
    tempOutput << output.str();  // Save header
    output.str("");
    output.clear();

    for (const auto& function : functions) {
        generateFunction(function.get());
    }

    std::string functionsCode = output.str();

    // Now build final output with proper ordering
    output.str("");
    output.clear();
    output << tempOutput.str();

    // Emit external declarations first
    emitExternalDeclarations();

    // Emit data section if we have any string literals or globals
    emitDataSection();

    // Text section for code
    output << "# Text section for executable code\n";
    output << ".text\n";
    output << "\n";

    // Emit all the function code
    output << functionsCode;

    // Add note at end
    output << "\n";
    output << "# End of generated assembly\n";

    return output.str();
}
