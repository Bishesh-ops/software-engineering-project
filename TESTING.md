# Testing Guide

Simple guide for running tests in the C compiler project.

## Quick Start

```bash
make test           # Run all tests
```

## Test Commands

```bash
make test           # Run both lexer and parser tests
make test_lexer     # Run only lexer tests
make test_parser    # Run only parser tests
```

## Build Commands

```bash
make                # Build everything
make clean          # Clean and rebuild
make all            # Build all test executables
```

## Test Files

### test_parser.cpp
Tests all parser functionality:
- ✓ Identifiers (`myVariable`, `_private123`)
- ✓ Literals (`42`, `3.14`)
- ✓ Binary operators (`+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`)
- ✓ Operator precedence (6 levels)
- ✓ Left-to-right associativity
- ✓ Complex nested expressions

### test_lexer.cpp
Tests tokenization of C source code.

## Understanding Test Output

```
[TEST] Simple Binary Operations
  ✓ a + b
  ✓ x * y

[TEST] Operator Precedence
  ✓ * has higher precedence than +
  ✓ Left associativity with different precedence
```

- `✓` = Test passed
- `✗` = Test failed (shows what failed)

## Operator Precedence Tested

1. `||` - Logical OR (lowest)
2. `&&` - Logical AND
3. `==`, `!=` - Equality
4. `<`, `>`, `<=`, `>=` - Relational
5. `+`, `-` - Additive
6. `*`, `/`, `%` - Multiplicative (highest)

## Example Workflow

```bash
# Quick test during development
make test_parser

# Full test before commit
make clean && make test
```

## Common Issues

**Tests won't compile**: Run `make clean` first

**Tests fail**: Check the specific test output for details
