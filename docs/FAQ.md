# Frequently Asked Questions (FAQ)

## Table of Contents

- [General Questions](#general-questions)
- [Installation & Setup](#installation--setup)
- [Usage Questions](#usage-questions)
- [Compilation Errors](#compilation-errors)
- [Runtime Errors](#runtime-errors)
- [Optimization Questions](#optimization-questions)
- [Debugging](#debugging)
- [Performance](#performance)
- [Contributing](#contributing)

---

## General Questions

### Q: What is this compiler?

**A:** This is an educational C compiler that translates a subset of C to x86-64 assembly. It includes:
- Full compilation pipeline (lexer → parser → semantic analysis → IR generation → optimization → code generation)
- SSA-based intermediate representation
- Multiple optimization passes
- System V AMD64 ABI compliance
- Debug symbol generation

### Q: What C features are supported?

**A:** Supported features include:
- ✅ Basic types: int, char, void, pointers, arrays, structs
- ✅ Operators: Arithmetic, comparison, logical, bitwise, assignment
- ✅ Control flow: if/else, while, for, do-while, break, continue, return
- ✅ Functions: declaration, definition, recursion
- ✅ Pointers: dereferencing, address-of, pointer arithmetic
- ✅ External functions: printf, malloc, math library

### Q: What platforms are supported?

**A:** Currently supported:
- ✅ macOS (primary development platform)
- ✅ Linux (tested on Ubuntu)
- ⏳ Windows (planned, via MinGW or WSL)

### Q: Is this a production compiler?

**A:** No, this is an **educational compiler** designed for learning compiler construction. For production use, consider:
- GCC (GNU Compiler Collection)
- Clang/LLVM
- MSVC (Microsoft Visual C++)

---

## Installation & Setup

### Q: What are the prerequisites?

**A:** You need:
```bash
# macOS
xcode-select --install  # Includes clang, make, as, ld

# Linux (Ubuntu/Debian)
sudo apt-get install build-essential g++ make binutils

# Required versions
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- GNU Make
- GNU Assembler (as)
```

### Q: How do I build the compiler?

**A:**
```bash
# Clone repository
git clone https://github.com/yourusername/c-compiler.git
cd c-compiler

# Build everything
make all

# Verify installation
make test
```

### Q: Build fails with "command not found"

**A:** Install missing tools:
```bash
# macOS
xcode-select --install

# Linux
sudo apt-get update
sudo apt-get install build-essential

# Verify tools
which g++    # Should show path to g++
which make   # Should show path to make
which as     # Should show path to assembler
```

### Q: Build fails with "C++17 required"

**A:** Update your compiler:
```bash
# Check version
g++ --version

# Need GCC 7+ or Clang 5+

# Ubuntu: Install newer GCC
sudo apt-get install g++-9
export CXX=g++-9

# macOS: Update Xcode Command Line Tools
xcode-select --install
```

---

## Usage Questions

### Q: How do I compile a C program?

**A:** Full workflow:
```bash
# Step 1: Generate assembly from C source
./bin/compiler source.c -o output.s

# Step 2: Assemble to object file
as -o output.o output.s

# Step 3: Link to executable
gcc -o output output.o

# Step 4: Run
./output
```

### Q: Can I use printf in my programs?

**A:** Yes! The compiler supports external function calls:
```c
// Use extern declaration
extern int printf(char* format, ...);

int main() {
    printf("Hello, World!\n");
    return 0;
}
```

Compile and link:
```bash
./compiler hello.c -o hello.s
as -o hello.o hello.s
gcc -o hello hello.o  # gcc links libc automatically
./hello  # Prints: Hello, World!
```

### Q: How do I enable optimizations?

**A:**
```bash
# Level 0: No optimization (fastest compile)
./compiler -O0 source.c -o output.s

# Level 1: Basic optimizations (default)
./compiler -O1 source.c -o output.s
# or just:
./compiler source.c -o output.s

# Level 2: Aggressive optimizations
./compiler -O2 source.c -o output.s
```

### Q: How do I enable debug symbols?

**A:**
```bash
# Compile with debug info
./compiler -g source.c -o output.s

# Assemble with debug info
as -g -o output.o output.s

# Link
gcc -o output output.o

# Debug
gdb output
```

---

## Compilation Errors

### Q: Error: "Syntax error: Expected ';'"

**A:** C statements must end with semicolons:
```c
// ❌ Wrong
int x = 5

// ✅ Correct
int x = 5;
```

### Q: Error: "Undeclared variable 'x'"

**A:** Variables must be declared before use:
```c
// ❌ Wrong
int main() {
    x = 5;  // x not declared
    return 0;
}

// ✅ Correct
int main() {
    int x;
    x = 5;
    return 0;
}
```

### Q: Error: "Type mismatch in assignment"

**A:** Cannot assign incompatible types:
```c
// ❌ Wrong
int x = "hello";  // string to int

// ✅ Correct
char* x = "hello";  // string to char*
```

### Q: Error: "Function 'foo' not declared"

**A:** Declare functions before use:
```c
// ❌ Wrong
int main() {
    foo();  // foo not declared
    return 0;
}
void foo() { }

// ✅ Correct - Declaration before use
void foo();

int main() {
    foo();
    return 0;
}

void foo() { }
```

### Q: Error: "Cannot apply operator '+' to pointer types"

**A:** Pointer arithmetic rules:
```c
int* p;
int* q;

// ❌ Wrong
int* r = p + q;  // Cannot add two pointers

// ✅ Correct
int* r = p + 5;  // Can add integer to pointer
int diff = p - q;  // Can subtract same-type pointers
```

---

## Runtime Errors

### Q: Segmentation fault when running program

**A:** Common causes:

**1. Uninitialized pointers:**
```c
// ❌ Wrong
int* p;
*p = 10;  // p not initialized!

// ✅ Correct
int x;
int* p = &x;
*p = 10;
```

**2. Array out of bounds:**
```c
// ❌ Wrong
int arr[5];
arr[10] = 0;  // Out of bounds!

// ✅ Correct
int arr[5];
arr[4] = 0;  // Last valid index
```

**3. Stack misalignment:**
- Our compiler handles this automatically
- If you modify assembly, ensure 16-byte alignment before `call`

### Q: Program returns wrong value

**A:** Check:

1. **Return statement:**
```c
// Return value becomes exit code (modulo 256)
int main() {
    return 42;  // Exit code will be 42
}
```

2. **Integer overflow:**
```c
// Exit codes are 0-255
return 300;  // Actual exit code: 300 % 256 = 44
```

3. **Uninitialized variables:**
```c
// ❌ Wrong
int x;
return x;  // Undefined behavior!

// ✅ Correct
int x = 0;
return x;
```

### Q: External function calls fail

**A:** Ensure proper linkage:
```bash
# ❌ Wrong - Missing libc
ld -o output output.o

# ✅ Correct - Use gcc to link
gcc -o output output.o

# Or link libc explicitly
ld -o output output.o -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2
```

---

## Optimization Questions

### Q: How do optimizations work?

**A:** The compiler has two optimization levels:

**IR-Level Optimizations:**
- Constant folding: `2 + 3` → `5`
- Dead code elimination: Remove unreachable code
- Common subexpression elimination: Reuse computed values

**Assembly-Level Optimizations:**
- Peephole optimization: Local instruction improvements
- Register allocation: Minimize memory access
- Multiply by power of 2 → shift: `x * 8` → `x << 3`

### Q: Can I see optimization statistics?

**A:** Yes, run with verbose flag:
```bash
./compiler -v source.c -o output.s
```

Output shows:
```
IR Optimization Report
======================
Constant Folding:           5 optimizations
Dead Code Elimination:      2 optimizations
Common Subexpression Elim.: 3 optimizations
----------------------------------------
Total Optimizations:        10
```

### Q: Why is my optimized code larger?

**A:** Sometimes optimizations trade size for speed:
- Loop unrolling increases size but reduces branches
- Inlining duplicates code but eliminates call overhead

Use `-Os` for size optimization (planned feature).

### Q: Can I disable specific optimizations?

**A:** Not individually in current version. Use:
- `-O0`: All optimizations off
- `-O1`: Basic optimizations on
- `-O2`: All optimizations on

---

## Debugging

### Q: How do I debug generated assembly?

**A:**
```bash
# Compile with debug symbols
./compiler -g source.c -o output.s

# Assemble with debug info
as -g -o output.o output.s

# Link
gcc -o output output.o

# Debug with gdb
gdb output
```

### Q: GDB shows no source code

**A:** Debug symbols must be enabled:
```bash
# Step 1: Compile with -g
./compiler -g source.c -o output.s

# Step 2: Assemble with -g  (IMPORTANT!)
as -g -o output.o output.s

# Step 3: Link
gcc -o output output.o

# Now gdb can show source
gdb output
(gdb) list  # Should show source code
```

### Q: How do I view generated assembly?

**A:**
```bash
# Generate assembly
./compiler source.c -o output.s

# View with syntax highlighting
cat output.s

# Or use less
less output.s

# Disassemble compiled binary
objdump -d output
```

### Q: Breakpoint in gdb doesn't work

**A:** Set breakpoint at function name:
```gdb
# At function
(gdb) break main
(gdb) break myfunction

# At source line (if debug symbols)
(gdb) break source.c:10

# At assembly label
(gdb) break *0x400500
```

---

## Performance

### Q: How fast is the generated code?

**A:** Performance characteristics:

**Compared to GCC -O0**: ~90-95% speed
**Compared to GCC -O2**: ~60-70% speed
**Compared to GCC -O3**: ~50-60% speed

Our compiler is educational, not production-quality.

### Q: How can I make my code faster?

**A:**

1. **Enable optimizations:**
```bash
./compiler -O2 source.c -o output.s
```

2. **Write optimization-friendly code:**
```c
// ✅ Good - Compiler can constant fold
int x = 2 + 3;

// ✅ Good - Compiler can CSE
int a = x + y;
int b = x + y;  // Reuses computation

// ❌ Less optimal - Prevents optimization
volatile int x = getValue();
```

3. **Use appropriate types:**
```c
// ✅ Fast - Native int type
int counter = 0;

// ❌ Slower - May require conversion
char counter = 0;  // Promoted to int for arithmetic
```

### Q: Why is compilation slow?

**A:** Optimization passes take time. Disable for faster compilation:
```bash
# Fastest compilation
./compiler -O0 source.c -o output.s

# Debug builds are faster
./compiler -g -O0 source.c -o output.s
```

---

## Contributing

### Q: How can I contribute?

**A:**
1. Fork the repository
2. Create feature branch: `git checkout -b feature/my-feature`
3. Write tests for new functionality
4. Ensure all tests pass: `make test`
5. Submit pull request

### Q: What should I work on?

**A:** See [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Open issues
- Feature requests
- Bug reports
- Documentation improvements

### Q: How do I report a bug?

**A:** Create an issue with:
1. **Minimal example** that reproduces the bug
2. **Expected behavior**
3. **Actual behavior**
4. **Compiler version** (`./compiler --version`)
5. **Platform** (macOS/Linux + version)

Example:
```
Title: Segfault on array initialization

Input code:
```c
int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    return arr[0];
}
```

Expected: Exit code 1
Actual: Segmentation fault

Platform: macOS 12.0, Apple clang 13.0
Compiler version: 1.0.0
```

---

## Additional Resources

- **Full Documentation**: [README.md](../README.md)
- **Code Generation Details**: [CODE_GENERATION.md](CODE_GENERATION.md)
- **Optimization Guide**: [../OPTIMIZATIONS_US8.md](../OPTIMIZATIONS_US8.md)
- **Testing Framework**: [../TESTING_FRAMEWORK_US9.md](../TESTING_FRAMEWORK_US9.md)
- **Example Programs**: [../examples/](../examples/)

---

**Still have questions?** Open an issue on GitHub!
