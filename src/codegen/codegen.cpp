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
// Code Generator - Implementation
// ============================================================================

CodeGenerator::CodeGenerator()
    : currentFunction(nullptr), stackFrameSize(0)
{
}

void CodeGenerator::reset()
{
    output.str("");
    output.clear();
    currentFunction = nullptr;
    stackFrameSize = 0;
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
    output << "    " << instruction << "\n";
}

void CodeGenerator::emitComment(const std::string& comment)
{
    output << "    # " << comment << "\n";
}

void CodeGenerator::emitLabel(const std::string& label)
{
    output << label << ":\n";
}

void CodeGenerator::emitPrologue()
{
    emitComment("Function prologue");
    emit("pushq %rbp");
    emit("movq %rsp, %rbp");

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
    emitComment("Function epilogue");
    emit("movq %rbp, %rsp");
    emit("popq %rbp");
    emit("ret");
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
            // Division is complex, requires special handling
            emitComment("Division operation");
            // TODO: Implement full division with RAX/RDX setup
            return;
        case IROpcode::MOD:
            // Modulo requires division
            emitComment("Modulo operation");
            // TODO: Implement full modulo with RAX/RDX setup
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

void CodeGenerator::emitCallInst(const IRInstruction* inst)
{
    const auto& operands = inst->getOperands();
    if (operands.empty()) return;

    std::string funcName = operands[0].getConstant();
    emitComment("Call function: " + funcName);

    // TODO: Handle argument passing according to System V ABI
    // Arguments in: RDI, RSI, RDX, RCX, R8, R9, then stack

    emit("call " + funcName);

    // Result in RAX
    if (inst->getResult()) {
        std::string dest = getRegisterForValue(inst->getResult());
        if (dest != "%rax") {
            emit("movq %rax, " + dest);
        }
    }
}

std::string CodeGenerator::generateFunction(IRFunction* function)
{
    currentFunction = function;

    // Step 1: Build live intervals
    allocator.buildLiveIntervals(function);

    // Step 2: Allocate registers
    allocator.allocate();

    // Step 3: Generate assembly
    output << "\n";
    emitComment("Function: " + function->getName());
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

                default:
                    emitComment("Unhandled opcode");
                    break;
            }
        }
    }

    return output.str();
}

std::string CodeGenerator::generateProgram(const std::vector<std::unique_ptr<IRFunction>>& functions)
{
    reset();

    // Emit assembly header
    output << "# Generated x86-64 assembly (AT&T syntax)\n";
    output << "# Target: System V AMD64 ABI\n";
    output << "\n";
    output << ".text\n";

    // Generate code for each function
    for (const auto& function : functions) {
        generateFunction(function.get());
    }

    return output.str();
}
