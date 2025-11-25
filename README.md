# C Compiler Project

A comprehensive C compiler implementation with lexical analysis, parsing, and semantic analysis phases.

## Project Structure

```
.
├── include/                  # Header files
│   ├── ast.h                # AST node definitions
│   ├── ast_printer.h        # AST visualization
│   ├── ast_visitor.h        # Visitor pattern interface
│   ├── lexer.h              # Lexical analyzer
│   ├── parser.h             # Syntax parser
│   ├── scope_manager.h      # Scope management
│   ├── semantic_analyzer.h  # Semantic analysis
│   ├── symbol_table.h       # Symbol table
│   ├── token.h              # Token definitions
│   └── type.h               # Type system
│
├── src/                      # Implementation files
│   ├── lexer/               # Lexer implementation
│   │   ├── lexer.cpp
│   │   └── token.cpp
│   ├── parser/              # Parser implementation
│   │   └── parser.cpp
│   ├── AST/                 # AST implementation
│   │   ├── ast_node.cpp
│   │   ├── ast_printer.cpp
│   │   └── ast_visitor.cpp
│   ├── nodes/               # AST node implementations
│   │   ├── declaration_nodes.cpp
│   │   ├── expression_nodes.cpp
│   │   └── statement_nodes.cpp
│   └── semantic/            # Semantic analysis implementation
│       ├── type.cpp
│       ├── symbol_table.cpp
│       ├── scope_manager.cpp
│       └── semantic_analyzer.cpp
│
├── test_lexer.cpp           # Lexer tests
├── test_parser.cpp          # Parser tests
├── test_semantic_main.cpp   # Main semantic tests (User Stories #1-10)
├── test_semantic_us11_implicit_conversions.cpp  # User Story #11
├── test_semantic_us12_pointer_arithmetic.cpp    # User Story #12
├── test_semantic_us13_struct_checking.cpp       # User Story #13
│
├── build/                   # Build artifacts (generated)
│   └── obj/                 # Object files
├── bin/                     # Executables (generated)
└── Makefile                 # Build configuration
```

## Building the Project

### Prerequisites
- C++17 compatible compiler (g++ recommended)
- make

### Build Commands

```bash
# Build all test executables
make all

# Clean build artifacts
make clean
```

## Running Tests

### Run All Tests
```bash
make test
```

### Run Individual Test Suites
```bash
# Lexer tests only
make test_lexer

# Parser tests only
make test_parser

# All semantic tests
make test_semantic
```

### Run Individual Executables
```bash
# Lexer tests
./bin/test_lexer.exe

# Parser tests
./bin/test_parser.exe

# Semantic tests
./bin/test_semantic_main.exe
./bin/test_semantic_us11.exe
./bin/test_semantic_us12.exe
./bin/test_semantic_us13.exe
```

## Test Files Overview

### test_lexer.cpp
Tests the lexical analyzer's ability to tokenize C source code:
- Keywords and identifiers
- Literals (integers, floats, strings, characters)
- Operators and punctuation
- Comments and whitespace handling
- Error handling for invalid tokens

### test_parser.cpp
Tests the parser's ability to construct Abstract Syntax Trees (AST):
- Variable declarations and initializations
- Function declarations and definitions
- Control flow statements (if, while, for)
- Expressions (arithmetic, logical, relational)
- Arrays and pointers
- Struct definitions

### test_semantic_main.cpp
Main semantic analysis tests covering User Stories #1-10:
- Basic type checking for variables and expressions
- Function type checking (parameters, return types)
- Scope management (block scopes, function scopes)
- Symbol table management
- Array and pointer type validation
- Control flow statement validation
- Struct declaration and initialization

### test_semantic_us11_implicit_conversions.cpp
**User Story #11: Implicit Type Conversions**

Tests automatic type conversions in expressions:
- Integer promotion (char → int, short → int)
- Arithmetic conversions (int + float → float)
- Type hierarchy: double > float > long > int > short > char
- Array-to-pointer decay (int[10] → int*)
- Insertion of TypeCastExpr nodes in AST

Example:
```c
char c = 'A';
int result = c + 1;  // char promoted to int
float f = 3.14;
float result2 = 5 + f;  // int converted to float
```

### test_semantic_us12_pointer_arithmetic.cpp
**User Story #12: Pointer Arithmetic Validation**

Tests pointer operation validation:
- Valid operations:
  - pointer + integer → pointer (scaled by pointee size)
  - pointer - integer → pointer (scaled by pointee size)
  - pointer - pointer → integer (same type only)
- Invalid operations (with specific error messages):
  - pointer * pointer
  - pointer * integer
  - pointer / pointer
  - pointer / integer
  - pointer % anything
  - pointer + pointer
  - different-type pointer subtraction
- Void pointer arithmetic restrictions

Example:
```c
int arr[10];
int *p = arr;
int *q = p + 5;      // Valid: pointer + integer
int diff = q - p;    // Valid: same-type pointer subtraction
int *invalid = p * 2; // Error: cannot multiply pointers
```

### test_semantic_us13_struct_checking.cpp
**User Story #13: Struct Type Checking**

Tests struct member access validation:
- Member access with dot operator (obj.member)
- Member access with arrow operator (ptr->member)
- Validation that object is struct type
- Validation that member exists in struct
- Member offset calculation
- Struct size calculation
- Error detection:
  - Non-existent members
  - Wrong operator (. vs ->)
  - Member access on non-struct types

Example:
```c
struct Point {
    int x;
    int y;
};

struct Point p;
p.x = 10;           // Valid: dot operator on struct
p.z = 5;            // Error: no member 'z'

struct Point *ptr = &p;
ptr->y = 20;        // Valid: arrow operator on pointer
ptr.x = 15;         // Error: should use -> for pointers
```

## Implemented Features

### Lexical Analysis
- Tokenization of C keywords, identifiers, literals, operators
- Source location tracking for error reporting
- Comment handling (single-line and multi-line)

### Parsing
- Recursive descent parser for C grammar subset
- AST construction with proper node types
- Expression precedence and associativity
- Declaration and statement parsing

### Semantic Analysis
- Type system with base types, pointers, arrays, and structs
- Symbol table with scope management
- Type checking for:
  - Variable declarations and assignments
  - Function calls and return values
  - Binary and unary operations
  - Array indexing and pointer dereferencing
  - Struct member access
- Implicit type conversions with AST transformation
- Pointer arithmetic validation
- Struct type checking with member validation

### Type System Features
- Base types: int, float, double, char, short, long, void
- Pointer types with multiple levels
- Array types with size tracking
- Struct types with member definitions
- Type conversions:
  - Integer promotion (char/short → int)
  - Arithmetic conversions (common type calculation)
  - Array-to-pointer decay
- Type operations:
  - Size calculation (getSizeInBytes)
  - Member offset calculation (getMemberOffset)
  - Type comparison and compatibility checking

### Error Reporting
- Detailed error messages with source locations
- Specific error messages for common mistakes
- Helpful suggestions (e.g., "did you mean '->'?")
- Multiple error collection per analysis pass

## Design Patterns

### Visitor Pattern
The project uses the Visitor pattern for AST traversal:
- `ASTVisitor` base class defines visit operations
- `SemanticAnalyzer` implements visitor for type checking
- `ASTPrinter` implements visitor for visualization
- Each AST node accepts visitors via `accept()` method

### Type System
- Shared ownership of type objects using `std::shared_ptr<Type>`
- Type registry for struct types in semantic analyzer
- Type factory methods (e.g., `Type::makeInt()`, `Type::makePointer()`)

### Symbol Table
- Scoped symbol management with `ScopeManager`
- Symbol lookup with scope chain traversal
- Support for shadowing and redeclaration detection

## Development Notes

### Compilation Flags
- `-Wall -Wextra`: Enable all warnings
- `-std=c++17`: C++17 standard
- `-g`: Debug symbols
- Platform-specific linker flags for macOS/Linux

### Makefile Targets
- `all`: Build all executables
- `test`: Run all tests (lexer, parser, semantic)
- `test_lexer`: Run only lexer tests
- `test_parser`: Run only parser tests
- `test_semantic`: Run all semantic tests
- `clean`: Remove build artifacts and executables
- `dirs`: Create build directories

## Testing Strategy

Each test file follows a consistent pattern:
1. Test case description
2. Input code as string
3. Expected result (pass/fail, error messages)
4. Automated verification
5. Summary reporting (X passed, Y failed)

All tests are designed to be:
- Self-contained (no external dependencies)
- Clear and readable
- Comprehensive (covering both valid and invalid cases)
- Maintainable (easy to add new tests)

## Typical Compilation Flow

1. **Lexer** - Source code → Tokens
2. **Parser** - Tokens → AST
3. **Semantic Analyzer** - AST → Validated AST (with symbol table)
4. **IR Generator** - AST → SSA-form IR
5. **Optimizer** - IR → Optimized IR (with constant folding)
6. **Code Generator** - IR → Target code (future enhancement)
7. **VM/Execution** - Run the generated code (future enhancement)

## Quick Start

```bash
# Clone the repository
cd software-engineering-project

# Build all tests
make all

# Run all tests
make test

# Run only semantic tests
make test_semantic

# Run optimization tests
make test_constant_folding

# Run optimization example
./bin/constant_folding_example.exe

# Clean and rebuild
make clean && make all
```

## Optimization Features

### Constant Folding

The compiler includes a **constant folding optimization** pass that evaluates constant expressions at compile time, reducing runtime computation.

**Examples:**
- `2 + 3` → `5` at compile time
- `10 * 0` → `0` at compile time
- Works on SSA-form IR

**Testing:**
```bash
make test_constant_folding
```

**Documentation:** See [CONSTANT_FOLDING_OPTIMIZATION.md](CONSTANT_FOLDING_OPTIMIZATION.md) for detailed documentation.

## Future Enhancements

Potential areas for expansion:
- **Code generation** (assembly/LLVM IR)
- **Additional optimization passes:**
  - Dead code elimination
  - Common subexpression elimination
  - Constant propagation
  - Algebraic simplifications
- More advanced type features (unions, enums, typedefs)
- Preprocessor support
- Error recovery in parser
- More comprehensive C standard library support

## Contributors

This project was developed as a software engineering project to demonstrate compiler construction techniques and best practices in C++ development.
