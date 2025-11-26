# Code Generation Details

This document provides comprehensive details on the x86-64 code generation phase of the C compiler.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Code Generation Pipeline](#code-generation-pipeline)
- [Instruction Selection](#instruction-selection)
- [Register Allocation](#register-allocation)
- [Stack Frame Management](#stack-frame-management)
- [Function Call Conventions](#function-call-conventions)
- [Assembly Output Format](#assembly-output-format)
- [External Function Calls](#external-function-calls)
- [Peephole Optimization](#peephole-optimization)
- [Debug Information](#debug-information)
- [Examples](#examples)

---

## Overview

The code generation phase translates optimized SSA-form intermediate representation (IR) into x86-64 assembly language. The generated assembly follows the **System V AMD64 ABI** calling convention used on Unix-like systems.

**Key Features**:
- x86-64 target architecture (AT&T syntax)
- System V AMD64 ABI compliance
- Linear scan register allocation
- Stack frame management with 16-byte alignment
- Support for external library calls
- Optional peephole optimization pass
- DWARF debug information generation

---

## Architecture

### Code Generation Components

```
IR (SSA Form)
    ↓
┌──────────────────────────────┐
│   CodeGenerator              │
│   - Instruction selection    │
│   - Register allocation      │
│   - Stack frame setup        │
│   - Assembly emission        │
└──────────────────────────────┘
    ↓
┌──────────────────────────────┐
│   LinearScanAllocator        │
│   - Live interval analysis   │
│   - Register assignment      │
│   - Spill code insertion     │
└──────────────────────────────┘
    ↓
┌──────────────────────────────┐
│   PeepholeOptimizer          │
│   - Pattern matching         │
│   - Local transformations    │
│   - Multi-pass optimization  │
└──────────────────────────────┘
    ↓
x86-64 Assembly (.s file)
```

### File Structure

- **include/codegen.h**: Code generator interface and classes
- **src/codegen/codegen.cpp**: Implementation (~2500 lines)
- **tests/test_codegen.cpp**: Code generation tests

---

## Code Generation Pipeline

### Phase 1: Initialization

```cpp
CodeGenerator codegen;
codegen.setPeepholeOptimization(true);  // Enable optimizations
```

### Phase 2: Function Processing

For each function in the IR:

1. **Function Prologue Generation**
   - Save frame pointer
   - Allocate stack space
   - Save callee-saved registers

2. **Register Allocation**
   - Build live intervals for SSA values
   - Assign physical registers
   - Insert spill code for register pressure

3. **Instruction Selection**
   - Map IR opcodes to x86-64 instructions
   - Handle addressing modes
   - Generate efficient instruction sequences

4. **Function Epilogue Generation**
   - Restore callee-saved registers
   - Deallocate stack space
   - Return to caller

5. **Peephole Optimization** (optional)
   - Pattern-based local optimizations
   - Multi-pass until convergence

### Phase 3: Assembly Emission

```cpp
string assembly = codegen.generateProgram(functions);
// Output: Complete assembly file ready for GNU assembler
```

---

## Instruction Selection

### IR to x86-64 Mapping

| IR Opcode | x86-64 Instruction | Notes |
|-----------|-------------------|-------|
| `ADD` | `addq` | 64-bit integer addition |
| `SUB` | `subq` | 64-bit integer subtraction |
| `MUL` | `imulq` | Signed 64-bit multiplication |
| `DIV` | `idivq` | Signed 64-bit division (quotient in %rax) |
| `MOD` | `idivq` | Signed 64-bit modulo (remainder in %rdx) |
| `EQ` | `cmpq + sete` | Set byte if equal |
| `NE` | `cmpq + setne` | Set byte if not equal |
| `LT` | `cmpq + setl` | Set byte if less than |
| `GT` | `cmpq + setg` | Set byte if greater than |
| `LE` | `cmpq + setle` | Set byte if less or equal |
| `GE` | `cmpq + setge` | Set byte if greater or equal |
| `LOAD` | `movq offset(%rbp)` | Load from memory |
| `STORE` | `movq %reg, offset(%rbp)` | Store to memory |
| `MOVE` | `movq` | Register/immediate move |
| `JMP` | `jmp` | Unconditional jump |
| `BRANCH` | `cmpq + je/jne/...` | Conditional branch |
| `CALL` | `call` | Function call |
| `RET` | `ret` | Return from function |
| `PARAM` | Register setup | Parameter passing |

### Arithmetic Instructions

**Example: Addition**

IR:
```
%3 = ADD %1, %2
```

Generated Assembly:
```assembly
movq    -8(%rbp), %rax    # Load %1 into %rax
addq    -16(%rbp), %rax   # Add %2 to %rax
movq    %rax, -24(%rbp)   # Store result as %3
```

**Example: Division**

IR:
```
%3 = DIV %1, %2
```

Generated Assembly:
```assembly
movq    -8(%rbp), %rax    # Load dividend into %rax
cqto                      # Sign-extend %rax into %rdx:%rax
movq    -16(%rbp), %rcx   # Load divisor into %rcx
idivq   %rcx              # Divide: quotient in %rax, remainder in %rdx
movq    %rax, -24(%rbp)   # Store quotient
```

### Comparison Instructions

**Example: Less Than**

IR:
```
%3 = LT %1, %2
```

Generated Assembly:
```assembly
movq    -8(%rbp), %rax    # Load %1
cmpq    -16(%rbp), %rax   # Compare with %2
setl    %al               # Set %al to 1 if less than
movzbq  %al, %rax         # Zero-extend to 64 bits
movq    %rax, -24(%rbp)   # Store result (0 or 1)
```

### Control Flow Instructions

**Example: Conditional Branch**

IR:
```
BRANCH %1, label_true, label_false
```

Generated Assembly:
```assembly
movq    -8(%rbp), %rax    # Load condition
cmpq    $0, %rax          # Compare with 0
jne     label_true        # Jump if non-zero
jmp     label_false       # Otherwise jump to false
```

---

## Register Allocation

### Linear Scan Algorithm

The compiler uses the **Linear Scan Register Allocation** algorithm (Poletto & Sarkar, 1999) for efficient register assignment.

#### Algorithm Steps

1. **Build Live Intervals**
   - For each SSA value, determine first and last use
   - Create interval [start, end]

2. **Sort by Start Point**
   - Sort intervals by start position

3. **Allocate Registers**
   - Maintain pool of available registers
   - When value becomes live:
     - If register available: Assign register
     - Else: Spill to stack

4. **Free Registers**
   - When interval ends, return register to pool

#### Available Registers

**Caller-saved** (temporary, not preserved across calls):
- `%rax`, `%rcx`, `%rdx`, `%rsi`, `%rdi`
- `%r8`, `%r9`, `%r10`, `%r11`

**Callee-saved** (must be preserved):
- `%rbx`, `%r12`, `%r13`, `%r14`, `%r15`

**Reserved**:
- `%rsp`: Stack pointer
- `%rbp`: Frame pointer

#### Register Pressure and Spilling

When all registers are in use, values must be **spilled** to the stack:

```assembly
# Value %5 spilled to stack
movq    %rax, -40(%rbp)   # Spill to memory

# Later reload
movq    -40(%rbp), %rax   # Reload from memory
```

---

## Stack Frame Management

### Stack Frame Layout

```
High Address
┌─────────────────┐
│  Return Address │  ← Pushed by CALL
├─────────────────┤
│  Old %rbp       │  ← Saved by function prologue
├─────────────────┤  ← %rbp (frame pointer)
│  Local Var 1    │  -8(%rbp)
├─────────────────┤
│  Local Var 2    │  -16(%rbp)
├─────────────────┤
│  Local Var 3    │  -24(%rbp)
├─────────────────┤
│  Spilled Values │  -32(%rbp), -40(%rbp), ...
├─────────────────┤
│  ...            │
├─────────────────┤  ← %rsp (stack pointer, 16-byte aligned)
Low Address
```

### Function Prologue

```assembly
pushq   %rbp              # Save old frame pointer
movq    %rsp, %rbp        # Set up new frame pointer
subq    $N, %rsp          # Allocate N bytes for locals
                          # N is rounded to 16-byte boundary
```

### Function Epilogue

```assembly
leave                     # Equivalent to: movq %rbp, %rsp; popq %rbp
ret                       # Return to caller
```

### 16-Byte Stack Alignment

The System V ABI requires the stack to be **16-byte aligned** before a `call` instruction.

**Compiler ensures**:
- Stack space allocation is multiple of 16 bytes
- Adjustments made if necessary

**Example**:
```c
// Need 20 bytes for locals
// Round up to 32 bytes for alignment
subq    $32, %rsp
```

---

## Function Call Conventions

### System V AMD64 ABI

The compiler follows the **System V AMD64 ABI** for function calls.

#### Integer/Pointer Arguments

First 6 arguments passed in registers:

| Argument | Register |
|----------|----------|
| 1st | `%rdi` |
| 2nd | `%rsi` |
| 3rd | `%rdx` |
| 4th | `%rcx` |
| 5th | `%r8` |
| 6th | `%r9` |

Arguments 7+ pushed onto stack (right-to-left).

#### Floating Point Arguments

First 8 FP arguments in `%xmm0` - `%xmm7`.

#### Return Values

- Integer/pointer: `%rax`
- Floating point: `%xmm0`

#### Register Preservation

**Caller-saved** (caller must save if needed):
- `%rax`, `%rcx`, `%rdx`, `%rsi`, `%rdi`, `%r8`-`%r11`

**Callee-saved** (callee must preserve):
- `%rbx`, `%rbp`, `%r12`-`%r15`

### Function Call Example

**C Code**:
```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(10, 20);
    return result;
}
```

**Generated Assembly**:
```assembly
    .text
    .globl add
add:
    pushq   %rbp
    movq    %rsp, %rbp

    # Parameters: a in %rdi, b in %rsi
    movq    %rdi, -8(%rbp)    # Save a to stack
    movq    %rsi, -16(%rbp)   # Save b to stack

    # return a + b
    movq    -8(%rbp), %rax
    addq    -16(%rbp), %rax

    leave
    ret

    .globl main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $16, %rsp

    # Call add(10, 20)
    movq    $10, %rdi         # First argument
    movq    $20, %rsi         # Second argument
    call    add               # Call function

    # Store return value
    movq    %rax, -8(%rbp)

    # return result
    movq    -8(%rbp), %rax

    leave
    ret
```

---

## Assembly Output Format

### File Structure

```assembly
    # Data section (constants, strings)
    .data
.LC0:
    .asciz "Hello, World!\n"

    # Text section (code)
    .text

    # Global function
    .globl main
main:
    # Function body
    pushq   %rbp
    movq    %rsp, %rbp
    # ...
    leave
    ret
```

### AT&T Syntax

The compiler generates **AT&T syntax** (used by GNU assembler):

| Feature | AT&T Syntax | Intel Syntax |
|---------|-------------|--------------|
| Operand Order | `op src, dst` | `op dst, src` |
| Register Prefix | `%rax` | `rax` |
| Immediate Prefix | `$42` | `42` |
| Memory Addressing | `offset(%base)` | `[base + offset]` |
| Size Suffix | `movq` (quadword) | `mov qword ptr` |

**Example**:
```assembly
# AT&T Syntax (our compiler)
movq    $42, %rax

# Intel Syntax (for reference)
mov     rax, 42
```

---

## External Function Calls

### Declaring External Functions

C code using external functions:
```c
extern int printf(char* format, ...);

int main() {
    printf("Hello!\n");
    return 0;
}
```

### Generated Assembly

```assembly
    .data
.LC0:
    .asciz "Hello!\n"

    .text
    .globl main
main:
    pushq   %rbp
    movq    %rsp, %rbp

    # Load string address into %rdi (first argument)
    leaq    .LC0(%rip), %rdi

    # Clear %rax (no vector registers used)
    xorq    %rax, %rax

    # Call printf via PLT
    call    printf@PLT

    # return 0
    movq    $0, %rax

    leave
    ret
```

### Position-Independent Code (PIC)

- External calls use `@PLT` (Procedure Linkage Table)
- String literals accessed via PC-relative addressing: `label(%rip)`

### Linking External Functions

```bash
# Assembly to object file
as -o program.o program.s

# Link with gcc (automatically links libc)
gcc -o program program.o

# Or link explicitly
ld -o program program.o -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2
```

---

## Peephole Optimization

### Overview

Peephole optimization performs **local transformations** on small sequences of assembly instructions.

**Enabled with**: `./bin/compiler -O2 source.c -o output.s`

### Optimization Patterns

#### 1. Redundant Move Elimination

**Before**:
```assembly
movq    %rax, %rax    # No-op
```

**After**: (instruction removed)

#### 2. Arithmetic with Zero

**Before**:
```assembly
addq    $0, %rax      # Adding zero
subq    $0, %rbx      # Subtracting zero
```

**After**: (instructions removed)

#### 3. Multiply by Power of 2

**Before**:
```assembly
imulq   $8, %rax      # Multiply by 8 (3+ cycles)
```

**After**:
```assembly
shlq    $3, %rax      # Shift left by 3 (1 cycle)
```

**Applies to**: 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

#### 4. Push/Pop Pair Elimination

**Before**:
```assembly
pushq   %rax
popq    %rax          # No net effect
```

**After**: (both instructions removed)

#### 5. Redundant Comparison Elimination

**Before**:
```assembly
cmpq    %rax, %rax    # Compare register with itself
```

**After**: (instruction removed, always equal)

### Multi-Pass Optimization

The peephole optimizer runs **up to 5 passes** until no more changes are detected:

```cpp
void PeepholeOptimizer::optimize() {
    bool changed = true;
    int passes = 0;
    const int MAX_PASSES = 5;

    while (changed && passes < MAX_PASSES) {
        changed = false;
        passes++;

        // Apply all optimization patterns
        // Set changed = true if any pattern matches
    }
}
```

### Example: Full Optimization

**Original Code**:
```assembly
movq    %rax, %rax        # Redundant
addq    $0, %rbx          # Add zero
imulq   $16, %rcx         # Multiply by 16
pushq   %rdx
popq    %rdx              # Push/pop pair
```

**After Peephole Optimization**:
```assembly
                          # Redundant move removed
                          # Add zero removed
shlq    $4, %rcx          # Multiply converted to shift
                          # Push/pop pair removed
```

---

## Debug Information

### DWARF Debug Symbols

When compiled with `-g` flag, the compiler generates **DWARF** debug information.

**Usage**:
```bash
./bin/compiler -g source.c -o output.s
as -g -o output.o output.s
gcc -o output output.o
gdb output
```

### Debug Directives

```assembly
    .file   "source.c"
    .loc    1 5 0        # File 1, Line 5, Column 0

main:
    .cfi_startproc       # Call Frame Information start
    pushq   %rbp
    .cfi_def_cfa_offset 16
    movq    %rsp, %rbp
    .cfi_offset 6, -16
    # ...
    leave
    .cfi_endproc         # Call Frame Information end
```

### GDB Integration

With debug symbols, GDB can:
- Show source code while stepping
- Set breakpoints at line numbers
- Display variable values
- Unwind stack traces

---

## Examples

### Example 1: Simple Function

**C Code**:
```c
int square(int x) {
    return x * x;
}
```

**Generated Assembly**:
```assembly
    .text
    .globl square
square:
    pushq   %rbp
    movq    %rsp, %rbp

    # Parameter x in %rdi
    movq    %rdi, -8(%rbp)

    # return x * x
    movq    -8(%rbp), %rax
    imulq   -8(%rbp), %rax

    leave
    ret
```

### Example 2: If Statement

**C Code**:
```c
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
```

**Generated Assembly**:
```assembly
    .text
    .globl max
max:
    pushq   %rbp
    movq    %rsp, %rbp

    movq    %rdi, -8(%rbp)     # Store a
    movq    %rsi, -16(%rbp)    # Store b

    # if (a > b)
    movq    -8(%rbp), %rax
    cmpq    -16(%rbp), %rax
    jg      .L_then

.L_else:
    # return b
    movq    -16(%rbp), %rax
    jmp     .L_end

.L_then:
    # return a
    movq    -8(%rbp), %rax

.L_end:
    leave
    ret
```

### Example 3: Loop

**C Code**:
```c
int sum(int n) {
    int total = 0;
    int i = 0;
    while (i < n) {
        total = total + i;
        i = i + 1;
    }
    return total;
}
```

**Generated Assembly**:
```assembly
    .text
    .globl sum
sum:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $32, %rsp

    movq    %rdi, -8(%rbp)     # Store n
    movq    $0, -16(%rbp)      # total = 0
    movq    $0, -24(%rbp)      # i = 0

.L_loop:
    # while (i < n)
    movq    -24(%rbp), %rax
    cmpq    -8(%rbp), %rax
    jge     .L_end

    # total = total + i
    movq    -16(%rbp), %rax
    addq    -24(%rbp), %rax
    movq    %rax, -16(%rbp)

    # i = i + 1
    movq    -24(%rbp), %rax
    addq    $1, %rax
    movq    %rax, -24(%rbp)

    jmp     .L_loop

.L_end:
    # return total
    movq    -16(%rbp), %rax

    leave
    ret
```

---

## References

- **System V AMD64 ABI**: [https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-1.0.pdf](https://github.com/hjl-tools/x86-psABI/wiki/x86-64-psABI-1.0.pdf)
- **x86-64 Instruction Reference**: [https://www.felixcloutier.com/x86/](https://www.felixcloutier.com/x86/)
- **Linear Scan Register Allocation**: Poletto & Sarkar, ACM TOPLAS 1999
- **DWARF Debugging Format**: [http://dwarfstd.org/](http://dwarfstd.org/)
- **AT&T Assembly Syntax**: [GNU Assembler Manual](https://sourceware.org/binutils/docs/as/)

---

## See Also

- [Main README](../README.md)
- [Optimization Guide](../OPTIMIZATIONS_US8.md)
- [Testing Framework](../TESTING_FRAMEWORK_US9.md)
- [FAQ](FAQ.md)
- [Example Programs](../examples/)

---

**For questions about code generation, consult this document or open an issue on GitHub.**
