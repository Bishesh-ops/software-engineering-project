// ============================================================================
// Example: Debug Symbol Generation
// ============================================================================
// Demonstrates the code generator's ability to emit debug information
// for use with gdb/lldb debuggers.
//
// This example shows:
// 1. .file directives for source file mapping
// 2. .loc directives for line number mapping
// 3. .cfi directives for stack frame unwinding
// 4. Function type information for debuggers

#include "../include/ir.h"
#include "../include/codegen.h"
#include <iostream>
#include <memory>
#include <fstream>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "Debug Symbol Generation Example\n";
    cout << "========================================\n\n";

    // ========================================================================
    // Example 1: Simple function with debug info disabled
    // ========================================================================

    cout << "Example 1: Normal code generation (no debug info)\n";
    cout << "===================================================\n\n";

    CodeGenerator codegen1;
    codegen1.setDebugMode(false);  // Debug mode OFF

    // Create a simple add function
    auto addFunc = make_unique<IRFunction>("add", "int");
    auto block = make_unique<IRBasicBlock>("entry");

    // Simple return statement
    block->addInstruction(make_unique<ReturnInst>(
        IROperand("42", IROperand::OperandType::CONSTANT)));

    addFunc->addBasicBlock(move(block));

    vector<unique_ptr<IRFunction>> functions1;
    functions1.push_back(move(addFunc));

    string assembly1 = codegen1.generateProgram(functions1);

    cout << "Generated Assembly (without debug info):\n";
    cout << "----------------------------------------\n";
    cout << assembly1 << "\n\n";

    // ========================================================================
    // Example 2: Same function with debug info enabled
    // ========================================================================

    cout << "Example 2: Code generation WITH debug info\n";
    cout << "============================================\n\n";

    CodeGenerator codegen2;
    codegen2.setDebugMode(true);   // Debug mode ON
    codegen2.setSourceFile("test.c");  // Set source file

    // Create the same add function
    auto addFunc2 = make_unique<IRFunction>("add", "int");
    auto block2 = make_unique<IRBasicBlock>("entry");

    block2->addInstruction(make_unique<ReturnInst>(
        IROperand("42", IROperand::OperandType::CONSTANT)));

    addFunc2->addBasicBlock(move(block2));

    vector<unique_ptr<IRFunction>> functions2;
    functions2.push_back(move(addFunc2));

    string assembly2 = codegen2.generateProgram(functions2);

    cout << "Generated Assembly (with debug info):\n";
    cout << "--------------------------------------\n";
    cout << assembly2 << "\n\n";

    // ========================================================================
    // Example 3: Complex function with multiple basic blocks
    // ========================================================================

    cout << "Example 3: Complex function with debug info\n";
    cout << "=============================================\n\n";

    CodeGenerator codegen3;
    codegen3.setDebugMode(true);
    codegen3.setSourceFile("calculator.c");

    // Create a function that does arithmetic
    auto calcFunc = make_unique<IRFunction>("calculate", "int");

    // Entry block
    auto entryBlock = make_unique<IRBasicBlock>("entry");

    // Create some SSA values
    SSAValue x("x", "int", 0);
    SSAValue y("y", "int", 0);
    SSAValue result("result", "int", 0);

    // Add instruction: result = x + y
    auto addInst = make_unique<ArithmeticInst>(
        IROpcode::ADD, &result,
        IROperand(x), IROperand(y)
    );
    entryBlock->addInstruction(move(addInst));

    // Return result
    entryBlock->addInstruction(make_unique<ReturnInst>(IROperand(result)));

    calcFunc->addBasicBlock(move(entryBlock));

    vector<unique_ptr<IRFunction>> functions3;
    functions3.push_back(move(calcFunc));

    string assembly3 = codegen3.generateProgram(functions3);

    cout << "Generated Assembly:\n";
    cout << "-------------------\n";
    cout << assembly3 << "\n\n";

    // ========================================================================
    // Save assembly to file for debugging with gdb
    // ========================================================================

    cout << "========================================\n";
    cout << "Saving Assembly for GDB Testing\n";
    cout << "========================================\n\n";

    // Save the assembly with debug info to a file
    ofstream outFile("debug_test.s");
    if (outFile.is_open()) {
        outFile << assembly3;
        outFile.close();
        cout << "✓ Saved assembly to: debug_test.s\n\n";
    } else {
        cout << "✗ Failed to save assembly file\n\n";
    }

    // ========================================================================
    // Debug Information Explanation
    // ========================================================================

    cout << "========================================\n";
    cout << "Debug Information Directives Explained\n";
    cout << "========================================\n\n";

    cout << "1. .file Directive\n";
    cout << "   Format: .file file_number \"filename\"\n";
    cout << "   Purpose: Maps source file to assembly\n";
    cout << "   Example: .file 1 \"test.c\"\n";
    cout << "   - Associates generated code with original C source\n";
    cout << "   - Debugger uses this to find source file\n\n";

    cout << "2. .loc Directive\n";
    cout << "   Format: .loc file_number line [column]\n";
    cout << "   Purpose: Maps assembly lines to source code lines\n";
    cout << "   Example: .loc 1 10 5\n";
    cout << "   - file 1 (from .file directive)\n";
    cout << "   - line 10 in source file\n";
    cout << "   - column 5 (optional)\n";
    cout << "   - Enables breakpoints at specific source lines\n\n";

    cout << "3. .type Directive\n";
    cout << "   Format: .type symbol_name, @function\n";
    cout << "   Purpose: Declares symbol type for debugger\n";
    cout << "   Example: .type calculate, @function\n";
    cout << "   - Tells debugger 'calculate' is a function\n";
    cout << "   - Helps with symbol resolution\n\n";

    cout << "4. CFI Directives (Call Frame Information)\n";
    cout << "   Purpose: Help debugger unwind stack frames\n";
    cout << "   Essential for backtraces and step debugging\n\n";

    cout << "   a) .cfi_startproc\n";
    cout << "      - Marks beginning of function\n";
    cout << "      - Starts CFI directive sequence\n\n";

    cout << "   b) .cfi_def_cfa_offset 16\n";
    cout << "      - Defines Canonical Frame Address offset\n";
    cout << "      - After 'pushq %rbp', CFA is at RSP+16\n\n";

    cout << "   c) .cfi_offset %rbp, -16\n";
    cout << "      - Indicates RBP was saved at CFA-16\n";
    cout << "      - Debugger can restore RBP value\n\n";

    cout << "   d) .cfi_def_cfa_register %rbp\n";
    cout << "      - CFA is now relative to RBP\n";
    cout << "      - After 'movq %rsp, %rbp'\n\n";

    cout << "   e) .cfi_endproc\n";
    cout << "      - Marks end of function\n";
    cout << "      - Closes CFI directive sequence\n\n";

    // ========================================================================
    // Using Debug Info with GDB
    // ========================================================================

    cout << "========================================\n";
    cout << "Using Generated Assembly with GDB\n";
    cout << "========================================\n\n";

    cout << "Step 1: Assemble with debug info\n";
    cout << "  $ as -g -o debug_test.o debug_test.s\n";
    cout << "  (The -g flag preserves debug information)\n\n";

    cout << "Step 2: Link to create executable\n";
    cout << "  $ ld -o debug_test debug_test.o\n";
    cout << "  or\n";
    cout << "  $ gcc -o debug_test debug_test.o\n\n";

    cout << "Step 3: Debug with GDB\n";
    cout << "  $ gdb debug_test\n\n";

    cout << "GDB Commands:\n";
    cout << "  (gdb) info functions       # List all functions\n";
    cout << "  (gdb) break calculate      # Set breakpoint at function\n";
    cout << "  (gdb) run                  # Run program\n";
    cout << "  (gdb) backtrace            # Show call stack\n";
    cout << "  (gdb) info frame           # Show current stack frame\n";
    cout << "  (gdb) disassemble          # Show assembly code\n";
    cout << "  (gdb) step                 # Step to next instruction\n";
    cout << "  (gdb) continue             # Continue execution\n\n";

    // ========================================================================
    // DWARF Debug Format Information
    // ========================================================================

    cout << "========================================\n";
    cout << "DWARF Debug Format\n";
    cout << "========================================\n\n";

    cout << "The debug directives generate DWARF debug information:\n\n";

    cout << "DWARF Sections:\n";
    cout << "  .debug_info      - Core debug information\n";
    cout << "  .debug_line      - Line number mappings (from .loc)\n";
    cout << "  .debug_frame     - Stack frame information (from .cfi)\n";
    cout << "  .debug_str       - String table for debug info\n";
    cout << "  .debug_abbrev    - Abbreviation table\n\n";

    cout << "When you assemble with 'as -g':\n";
    cout << "  1. .file and .loc directives create .debug_line section\n";
    cout << "  2. .cfi directives create .debug_frame section\n";
    cout << "  3. .type directives add symbol information\n";
    cout << "  4. Debuggers read these sections to provide:\n";
    cout << "     - Source line mapping\n";
    cout << "     - Stack backtraces\n";
    cout << "     - Variable information\n";
    cout << "     - Function boundaries\n\n";

    // ========================================================================
    // Benefits of Debug Information
    // ========================================================================

    cout << "========================================\n";
    cout << "Benefits of Debug Symbol Generation\n";
    cout << "========================================\n\n";

    cout << "1. Source-Level Debugging\n";
    cout << "   - Set breakpoints at source lines\n";
    cout << "   - Step through C code (not just assembly)\n";
    cout << "   - See original variable names\n\n";

    cout << "2. Stack Unwinding\n";
    cout << "   - Accurate backtraces in gdb\n";
    cout << "   - Exception handling (C++)\n";
    cout << "   - Crash analysis\n\n";

    cout << "3. Performance Profiling\n";
    cout << "   - Tools like 'perf' use debug info\n";
    cout << "   - Map performance data to source lines\n";
    cout << "   - Identify hotspots in original code\n\n";

    cout << "4. Production Debugging\n";
    cout << "   - Core dump analysis\n";
    cout << "   - Post-mortem debugging\n";
    cout << "   - Remote debugging\n\n";

    // ========================================================================
    // Implementation Summary
    // ========================================================================

    cout << "========================================\n";
    cout << "Implementation Summary\n";
    cout << "========================================\n\n";

    cout << "Code Generator Enhancements:\n";
    cout << "  ✓ debugMode flag to enable/disable debug info\n";
    cout << "  ✓ .file directive for source file mapping\n";
    cout << "  ✓ .loc directives for line number mapping\n";
    cout << "  ✓ .type directives for function symbols\n";
    cout << "  ✓ .cfi directives for stack unwinding\n";
    cout << "  ✓ Optional debug info (no overhead when disabled)\n\n";

    cout << "Public API:\n";
    cout << "  codegen.setDebugMode(true/false);   // Enable/disable\n";
    cout << "  codegen.setSourceFile(\"file.c\");    // Set source file\n";
    cout << "  codegen.isDebugMode();              // Check if enabled\n\n";

    cout << "========================================\n";
    cout << "Example Complete!\n";
    cout << "========================================\n";

    return 0;
}
