# Software-Engineering-Project
  include/

  Public header files (.h) - declares classes, functions, and interfaces:
  - lexer.h - Lexer class interface (tokenization)
  - token.h - Token types and Token class definition
  - parser.h - Parser class interface (syntax analysis)
  - ast.h - AST node class declarations
  - semantic.h - Semantic analyzer interface
  - symbol_table.h - Symbol table for tracking variables/functions
  - type.h - Type system (int, char, pointer, struct, etc.)
  - ir.h - Intermediate representation structures
  - codegen.h - Code generator interface
  - vm.h - Virtual machine interface (if using bytecode)
  - error_handler.h - Error reporting utilities
  - preprocessor.h - Preprocessor directives handler (optional)

  src/

  Implementation files (.cpp) - actual code:

  src/main.cpp

  - Compiler entry point
  - Orchestrates: read file → lex → parse → semantic analysis → IR → codegen → output

  src/lexer/

  - lexer.cpp - Converts source code into tokens
  - token.cpp - Token class implementation
  - preprocessor.cpp - Handles #include, #define (optional)

  src/parser/

  - parser.cpp - Main parser logic, constructs AST from tokens
  - expression_parser.cpp - Parses expressions (arithmetic, logical, etc.)
  - statement_parser.cpp - Parses statements (if, while, for, return, etc.)

  src/ast/

  Abstract Syntax Tree - tree representation of source code:
  - ast_node.cpp - Base AST node class
  - ast_visitor.cpp - Visitor pattern for traversing AST
  - ast_printer.cpp - Debug tool to visualize AST structure
  - nodes/ - Specific node types:
    - expression_nodes.cpp - Binary ops, unary ops, literals, identifiers
    - statement_nodes.cpp - If, while, for, return statements
    - declaration_nodes.cpp - Variable and type declarations
    - function_nodes.cpp - Function definitions and calls

  src/semantic/

  Semantic analysis - ensures code makes sense:
  - semantic_analyzer.cpp - Main semantic checking orchestrator
  - symbol_table.cpp - Tracks declared variables/functions and their types
  - scope_manager.cpp - Manages nested scopes (global, function, block)
  - type_checker.cpp - Verifies type compatibility in operations
  - type.cpp - Type system implementation

  src/ir/

  Intermediate Representation - platform-independent code representation:
  - ir_generator.cpp - Converts AST to IR
  - ir_instruction.cpp - IR instruction definitions
  - ir_optimizer.cpp - Optimizations (constant folding, dead code elimination)
  - ir_printer.cpp - Debug tool to print IR

  src/codegen/

  Code generation - produces final output:
  - codegen.cpp - Main code generation logic
  - bytecode_generator.cpp - Generates bytecode for VM
  - assembly_generator.cpp - Generates x86/ARM assembly
  - c_generator.cpp - Transpiles to C (easiest backend option)

  src/vm/

  Virtual Machine (if using bytecode approach):
  - vm.cpp - VM execution engine
  - bytecode.cpp - Bytecode format definitions
  - instruction_set.cpp - Bytecode instruction implementations
  - runtime.cpp - Runtime library (I/O, memory management)

  src/utils/

  Helper utilities:
  - error_handler.cpp - Pretty error messages with line numbers
  - file_reader.cpp - File I/O utilities
  - string_utils.cpp - String manipulation helpers
  - debug_utils.cpp - Debug/logging utilities

  tests/

  Testing infrastructure:

  tests/unit/

  Unit tests - test individual components in isolation:
  - test_lexer.cpp - Test tokenization
  - test_parser.cpp - Test AST generation
  - test_semantic.cpp - Test type checking, symbol resolution
  - test_ir.cpp - Test IR generation
  - test_codegen.cpp - Test code output
  - test_vm.cpp - Test VM execution

  tests/integration/

  Integration tests - test multiple components together:
  - test_e2e.cpp - End-to-end: source code → execution
  - test_pipeline.cpp - Test full compilation pipeline

  tests/fixtures/

  Test input/output files:
  - valid/ - Correct C programs that should compile
  - invalid/ - Programs with errors (for testing error handling)
  - expected/ - Expected output from running compiled programs

  tests/test_runner.sh

  Script to run all tests automatically

  examples/

  Sample programs demonstrating compiler features:
  - Progressive complexity from hello world to advanced features
  - Used for manual testing and demonstrations

  lib/

  Third-party libraries:
  - Testing frameworks (Catch2, Google Test)
  - Any external dependencies

  build/

  Build output (should be in .gitignore):
  - obj/ - Compiled object files (.o)
  - bin/ - Final executables
  - lib/ - Compiled libraries

  scripts/

  Automation scripts:
  - build.sh - Compile the compiler
  - test.sh - Run test suite
  - benchmark.sh - Performance measurements
  - format.sh - Code formatting (clang-format)
  - generate_docs.sh - Generate API documentation (Doxygen)

  benchmarks/

  Performance testing:
  - benchmark_lexer.cpp - Measure lexer speed
  - benchmark_parser.cpp - Measure parser speed
  - large_programs/ - Large test files to stress-test compiler

  ---
  Typical compilation flow:
  1. Lexer - Source code → Tokens
  2. Parser - Tokens → AST
  3. Semantic Analyzer - AST → Validated AST (with symbol table)
  4. IR Generator - AST → IR
  5. Optimizer - IR → Optimized IR (optional)
  6. Code Generator - IR → Target code (bytecode/assembly/C)
  7. VM/Execution - Run the generated code


  Building and Running the Project (Using Make)

    This project uses a `Makefile` to automate the build process.  
    You can build, test, and clean the project easily from the terminal.

  Requirements
    Make sure you have:
    - **g++ (C++17 or higher)**
    - **GNU Make** (available in most development environments)
    - **PowerShell**, **Git Bash**, or a **Linux/macOS Terminal**

  Common Commands

    | Command | Description |
    |----------|-------------|
    | `make` | Builds the compiler and all dependencies |
    | `make test` | Builds and runs the lexer test executable |
    | `make clean` | Deletes all compiled files and build directories (`build/` and `bin/`) |

  Example Usage

```bash
# Build the project
make

# Run the lexer tests
make test

# Clean all build artifacts
make clean

```

After a successful build:

  build/obj/   -> Compiled object files (.o)
  bin/         -> Executables (.exe)
