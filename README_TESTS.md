# Test Files Overview

## Current Test Files

### test_parser.cpp ⭐
**The main parser test file** - Tests all parser functionality in one clean file.

- Simple helper functions for easy testing
- Clear, readable test output
- Covers all acceptance criteria for User Stories #2 and #3

**What it tests:**
- Identifier parsing
- Literal parsing
- Binary expressions with all operators
- Operator precedence (6 levels)
- Left-to-right associativity
- Complex nested expressions

### test_lexer.cpp
Tests the lexer/tokenizer functionality.

### test_ast.cpp
Tests AST node creation (standalone).

## Running Tests

```bash
make test           # Run all tests
make test_parser    # Run only parser tests
make test_lexer     # Run only lexer tests
```

## Test Output Example

```
[TEST] Simple Binary Operations
  [PASS] a + b
  [PASS] x * y

[TEST] All Operators
  [PASS] All 12 operators work
```

Simple and clean! [PASS] = pass, [FAIL] = fail

## Why One Test File?

- **Easier to maintain** - All tests in one place
- **Easier to understand** - Simple helper functions
- **Faster to run** - Single executable
- **Same coverage** - Tests everything the old files did

## File Structure

```
test_parser.cpp
├── Helper functions (parse, isIdentifier, isLiteral, etc.)
├── test_identifiers()
├── test_literals()
├── test_simple_binary_ops()
├── test_precedence()
├── test_associativity()
├── test_all_operators()
├── test_complex_expressions()
├── test_precedence_levels()
└── test_comparison_operators()
```

Each test is focused and easy to understand!
