# C Compiler Examples

This directory contains example C programs demonstrating the compiler's capabilities, along with expected assembly output.

## Example Programs

| File | Description | Demonstrates |
|------|-------------|--------------|
| `01_simple_arithmetic.c` | Basic arithmetic operations | Variable declaration, addition, return values |
| `02_function_call.c` | Function definitions and calls | Parameter passing, System V ABI, multiple functions |
| `03_control_flow.c` | If/else and recursion | Conditional branches, recursive calls, factorial |
| `04_external_printf.c` | External library functions | Calling printf, string literals, variadic functions |
| `05_optimizations.c` | Compiler optimizations | Constant folding, dead code elimination, peephole opts |

## How to Use These Examples

### Basic Compilation

```bash
# Step 1: Compile C source to assembly
./bin/compiler examples/01_simple_arithmetic.c -o output.s

# Step 2: Assemble to object file
as -o output.o output.s

# Step 3: Link to executable
gcc -o output output.o

# Step 4: Run the program
./output
echo $?  # Print exit code
```

### One-Line Compilation

```bash
# All steps in one command
./bin/compiler examples/01_simple_arithmetic.c -o output.s && \
  as -o output.o output.s && \
  gcc -o output output.o && \
  ./output
```

### Testing All Examples

```bash
# Compile and test all examples
for example in examples/0*.c; do
    echo "Testing $example..."
    name=$(basename "$example" .c)
    ./bin/compiler "$example" -o "/tmp/$name.s" && \
      as -o "/tmp/$name.o" "/tmp/$name.s" && \
      gcc -o "/tmp/$name" "/tmp/$name.o" && \
      "/tmp/$name"
    echo "Exit code: $?"
    echo "---"
done
```

## Viewing Assembly Output

### View Generated Assembly

```bash
# Generate assembly
./bin/compiler examples/01_simple_arithmetic.c -o output.s

# View with syntax highlighting (if available)
cat output.s

# Or use less for paging
less output.s
```

### Compare Optimization Levels

```bash
# Generate with different optimization levels
./bin/compiler -O0 examples/05_optimizations.c -o output_O0.s
./bin/compiler -O1 examples/05_optimizations.c -o output_O1.s
./bin/compiler -O2 examples/05_optimizations.c -o output_O2.s

# Compare file sizes
wc -l output_O*.s

# View differences
diff output_O0.s output_O2.s
```

## Example Output Formats

### Exit Code Testing

Many examples return specific values that can be checked via exit codes:

```bash
./output
echo $?  # Prints exit code (0-255)
```

**Note**: Exit codes are modulo 256, so `return 300` becomes `44`.

### Printf Output Testing

Examples using `printf` produce text output:

```bash
./bin/compiler examples/04_external_printf.c -o output.s
as -o output.o output.s
gcc -o output output.o
./output
# Output:
# Hello, World!
# The answer is: 42
```

## Understanding Assembly Output

### Assembly Structure

```assembly
    .data               # Data section (constants, strings)
.LC0:
    .asciz "Hello!\n"

    .text               # Code section
    .globl main         # Make 'main' visible to linker
main:
    pushq   %rbp        # Function prologue
    movq    %rsp, %rbp  # Set up stack frame

    # Function body
    movq    $42, %rax   # Put 42 in return register

    leave               # Function epilogue
    ret                 # Return to caller
```

### Key Assembly Concepts

**Registers** (x86-64 / System V ABI):
- `%rax`: Return value, general purpose
- `%rdi, %rsi, %rdx, %rcx, %r8, %r9`: First 6 integer arguments
- `%rbp`: Base pointer (stack frame)
- `%rsp`: Stack pointer

**Instructions**:
- `movq`: Move quadword (64-bit)
- `addq/subq`: Add/subtract
- `imulq`: Signed multiply
- `idivq`: Signed divide
- `cmpq`: Compare
- `jmp/je/jg/jl`: Unconditional/conditional jumps
- `call/ret`: Function call/return

**Stack Frame**:
- Local variables stored at negative offsets from `%rbp`
- Example: `-8(%rbp)` is first local variable
- Stack grows downward (toward lower addresses)

## Optimization Examples

### Constant Folding

**Before**: `int x = 2 + 3;`
```assembly
movq    $2, %rax
addq    $3, %rax    # Runtime computation
movq    %rax, -8(%rbp)
```

**After**: `int x = 5;`
```assembly
movq    $5, -8(%rbp)  # Compile-time computation
```

### Multiply to Shift

**Before**: `int y = x * 8;`
```assembly
movq    -8(%rbp), %rax
imulq   $8, %rax      # Multiply (3+ cycles)
```

**After**: `int y = x << 3;`
```assembly
movq    -8(%rbp), %rax
shlq    $3, %rax      # Shift (1 cycle)
```

### Dead Code Elimination

**Before**:
```c
int x = 42;
return x;
int y = 100;  // Dead code
return y;
```

**After**:
```c
int x = 42;
return x;
// Dead code removed entirely
```

## Debugging Assembly

### Using GDB

```bash
# Compile with debug symbols
./bin/compiler -g examples/03_control_flow.c -o output.s
as -g -o output.o output.s
gcc -o output output.o

# Debug with gdb
gdb output
(gdb) break main
(gdb) run
(gdb) step
(gdb) info registers
(gdb) disassemble main
```

### Disassemble Binary

```bash
# Disassemble compiled binary
objdump -d output

# Show specific function
objdump -d output | grep -A 20 "^<main>:"
```

## Common Issues

### Issue: Segmentation Fault

**Cause**: Stack misalignment or uninitialized pointers

**Solution**: Our compiler ensures 16-byte stack alignment. Check for:
- Uninitialized pointer dereferencing
- Array bounds violations

### Issue: Wrong Exit Code

**Cause**: Exit codes are modulo 256

**Example**:
```c
return 300;  // Exit code: 300 % 256 = 44
```

### Issue: Printf Output Not Appearing

**Cause**: Output buffering

**Solution**: Add newline or use:
```c
extern int fflush(void*);
printf("Hello");
fflush(0);  // Flush stdout
```

## Further Reading

- **Main Documentation**: [../README.md](../README.md)
- **Code Generation Details**: [../docs/CODE_GENERATION.md](../docs/CODE_GENERATION.md)
- **Optimization Guide**: [../OPTIMIZATIONS_US8.md](../OPTIMIZATIONS_US8.md)
- **Testing Framework**: [../TESTING_FRAMEWORK_US9.md](../TESTING_FRAMEWORK_US9.md)
- **FAQ**: [../docs/FAQ.md](../docs/FAQ.md)

## Next Steps

1. Try compiling each example
2. Examine the generated assembly
3. Experiment with different optimization levels
4. Modify the examples and see how assembly changes
5. Write your own C programs!

---

**Happy compiling!**
