# C Compiler Project - Context for Claude

## Project Overview
This is a **C compiler implementation** built from scratch as a software engineering project. The compiler is being developed in C++17 and follows a traditional multi-phase compilation architecture.

**Current Phase**: AST (Abstract Syntax Tree) Design and Implementation
**Active Branch**: `designAST`
**Main Branch**: `main`

---

## Project Architecture

### Compilation Pipeline
```
Source Code → Lexer → Parser → AST → Semantic Analysis → IR → Code Generation → Output
```

### Current Implementation Status

#### ✅ Completed Components

**1. Lexer (Tokenization)**
- Location: `src/lexer/`, `include/lexer.h`
- Fully functional lexical analyzer
- Supports all C89 keywords (32 keywords)
- Handles identifiers, literals (int, float, string, char)
- Operators: arithmetic, comparison, logical, bitwise, compound assignment
- Preprocessor tokens (#, ##)
- Error recovery with max error limit
- Zero-copy optimization using `string_view`
- Line directive handling for error reporting

**2. AST Node Design** ⭐ *Recently Completed*
- Location: `include/ast.h`, `src/AST/`, `src/nodes/`
- Complete AST node hierarchy
- Visitor pattern implementation
- Source location tracking for error reporting
- Memory management via `std::unique_ptr`

#### 🚧 In Progress

**Parser Implementation**
- Location: `src/parser/`, `include/parser.h`
- Structure in place but not fully implemented
- Subdirectories:
  - `expression_parser.cpp` - Expression parsing
  - `statement_parser.cpp` - Statement parsing
  - `declaration_parser.cpp` - Declaration parsing
  - `typer_parser.cpp` - Type parsing
  - `parser.cpp` - Main parser orchestration

#### 📋 Planned (Not Yet Started)

- Semantic Analysis
- Symbol Table
- Type Checking
- Intermediate Representation (IR)
- Code Generation
- Virtual Machine / Backend

---

## AST Node Structure

### Node Hierarchy

```
ASTNode (base class)
├── Expression
│   ├── BinaryExpr (e.g., a + b, x * y)
│   ├── UnaryExpr (e.g., -x, !flag, *ptr, &var, ++i)
│   ├── LiteralExpr (42, 3.14, "hello", 'c', true)
│   ├── IdentifierExpr (variable/function names)
│   └── CallExpr (function calls)
├── Statement
│   ├── IfStmt (if-else)
│   ├── WhileStmt
│   ├── ForStmt
│   ├── ReturnStmt
│   └── CompoundStmt (block of statements)
└── Declaration
    ├── VarDecl (variable declarations)
    ├── TypeDecl (typedef)
    ├── StructDecl
    ├── FunctionDecl
    └── ParameterDecl
```

### Key Design Decisions

1. **Source Location Tracking**: Every AST node stores `SourceLocation` (filename, line, column) for precise error reporting
2. **Visitor Pattern**: All nodes implement `accept(ASTVisitor&)` for traversal
3. **Smart Pointers**: All child nodes owned via `std::unique_ptr` for automatic memory management
4. **Type Safety**: Strong typing with `ASTNodeType` enum for node identification

### AST Files

- `include/ast.h` - All AST class definitions (header-only implementation)
- `src/AST/ast_node.cpp` - Base node implementations
- `src/AST/ast_visitor.cpp` - Visitor pattern base
- `src/AST/ast_printer.cpp` - Debug printer for visualizing AST
- `src/nodes/expression_nodes.cpp` - Expression node documentation
- `src/nodes/statement_nodes.cpp` - Statement node implementations
- `src/nodes/declaration_nodes.cpp` - Declaration node implementations

---

## Build System

**Build Tool**: GNU Make
**Compiler**: g++ (C++17 or higher)
**Platform**: macOS (Darwin 25.0.0)

### Common Commands

```bash
make           # Build the compiler and all dependencies
make test      # Build and run tests
make clean     # Remove all build artifacts (build/, bin/)
```

### Directory Structure

```
build/obj/     # Compiled object files (.o)
bin/           # Executables
```

---

## Testing

### Test Files

- `test_lexer.cpp` - Lexer unit tests
- `test_ast.cpp` - AST node tests (recently created)
  - Tests function declarations
  - Tests variable declarations
  - Tests if statements
  - Demonstrates visitor pattern usage

### Test Example

The current AST test creates and validates:
```c
int add(int a, int b) { return a + b; }
int x = 42;
if (x > 0) return x; else return -x;
```

---

## Git Workflow

### Branches

- `main` - Main development branch (use for PRs)
- `designAST` - Current working branch (AST implementation)
- `lexer`, `parser` - Other feature branches
- Team member branches: `lexer_acley`, `lexer_bishesh`, `lexer_naomi`, etc.

### Repository

**URL**: https://github.com/Bishesh-ops/software-engineering-project
**Remote**: origin

### Recent Commits (designAST branch)

```
a8a97ad - design ast node
977bcfb - Added ast header file
7335b79 - Updating File Structure
32b4f25 - Added new header files
```

---

## Code Style & Conventions

### Header Guards
```cpp
#ifndef FILE_H
#define FILE_H
// ...
#endif
```

### Memory Management
- Use `std::unique_ptr` for ownership
- Use raw pointers only for non-owning references (getter methods)
- Automatic cleanup via RAII

### Naming Conventions
- Classes: PascalCase (e.g., `BinaryExpr`, `ASTVisitor`)
- Functions/Methods: camelCase (e.g., `getLeft()`, `accept()`)
- Members: camelCase with trailing underscore for private members (e.g., `source_`)
- Enums: SCREAMING_SNAKE_CASE (e.g., `BINARY_EXPR`)

---

## Important Files to Reference

### Core Headers
- `include/ast.h` - Complete AST node definitions
- `include/lexer.h` - Lexer interface and Token types
- `include/parser.h` - Parser interface (minimal currently)
- `include/type.h` - Type system
- `include/source_location.h` - Source location utilities

### Core Implementation
- `src/lexer/lexer.cpp` - Lexical analysis implementation
- `src/lexer/token.cpp` - Token class implementation
- `src/AST/ast_printer.cpp` - AST visualization (useful for debugging)

### Documentation
- `README.md` - Comprehensive project structure documentation
- `src/nodes/expression_nodes.cpp` - Detailed expression node documentation
- This file (`claude.md`) - Context for AI assistance

---

## Next Steps / TODO

Based on current progress, the next logical steps are:

1. **Complete Parser Implementation**
   - Implement expression parsing with proper operator precedence
   - Implement statement parsing (if, while, for, return, etc.)
   - Implement declaration parsing (variables, functions, structs)
   - Connect parser to lexer and generate AST nodes

2. **Parser Testing**
   - Create comprehensive parser tests
   - Test error recovery and reporting
   - Validate AST structure for various C constructs

3. **Semantic Analysis** (Future)
   - Symbol table implementation
   - Type checking
   - Scope management
   - Semantic validation

---

## Team Context

This appears to be a collaborative project with multiple team members:
- Acley (current user)
- Bishesh (repository owner)
- Naomi
- Others

Each team member has worked on different branches, particularly around the lexer implementation.

---

## Common Issues & Notes

### Git Identity Configuration
The commit author is currently set as `acley <mac@Acleys-mac-4.local>`. To set proper Git identity:

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

### Include Paths
When including project headers, use relative paths:
```cpp
#include "include/ast.h"     // From project root
#include "../../include/ast.h"  // From deep nested files
```

---

## Quick Reference: Key Classes

### Token (Lexer Output)
```cpp
struct Token {
    TokenType type;
    string_view value;        // Zero-copy view
    string processed_value;   // For literals
    string filename;
    int line, column;
}
```

### ASTNode (Base Class)
```cpp
class ASTNode {
    SourceLocation location;
    ASTNodeType nodeType;
    virtual void accept(ASTVisitor& visitor) = 0;
}
```

### Expression Examples
```cpp
BinaryExpr(left, op, right, loc)      // a + b
UnaryExpr(op, operand, prefix, loc)   // -x, ++i
LiteralExpr(value, type, loc)         // 42, "hello"
IdentifierExpr(name, loc)             // variable_name
CallExpr(callee, args, loc)           // foo(a, b)
```

---

## Useful Commands for Claude

```bash
# Build and test
make && make test

# Check AST structure
./bin/test_ast

# View token output
./bin/test_lexer

# Find all AST-related files
find . -name "*ast*" -type f

# Search for specific node types
grep -r "BinaryExpr" include/ src/
```

---

*Last Updated: 2025-11-15*
*Current Working Directory: /Users/mac/software-engineering-project*
