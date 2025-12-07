# mycc Compiler - Testing & QA Documentation

## 📋 Table of Contents
1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Running Tests](#running-tests)
4. [Test Organization](#test-organization)
5. [Writing New Tests](#writing-new-tests)
6. [Testing Standards](#testing-standards)
7. [CI/CD Integration](#cicd-integration)
8. [Coverage Reports](#coverage-reports)

---

## Overview

This directory contains the comprehensive test suite for the **mycc** C compiler project. The testing infrastructure ensures code quality, correctness, and maintainability across all layers of the system:

- **C++ Compiler Backend** (Lexer, Parser, Semantic Analysis, IR, Codegen)
- **Python Flask API** (Bridge layer)
- **React Frontend** (Visualization and UI)

### Testing Philosophy

- **Test-First Approach:** Write tests before or alongside production code
- **Comprehensive Coverage:** Aim for >80% code coverage
- **Fast Feedback:** Unit tests run in milliseconds
- **Isolated Tests:** Each test is independent and deterministic
- **Readable Tests:** Tests serve as documentation

---

## Architecture

### Test Directory Structure

```
tests/
├── backend/                    # C++ compiler tests
│   ├── unit/                   # Unit tests for individual components
│   │   ├── lexer/              # Lexical analysis tests
│   │   ├── parser/             # Syntax parsing tests
│   │   ├── semantic/           # Semantic analysis tests
│   │   ├── ir/                 # IR generation tests
│   │   ├── codegen/            # Code generation tests
│   │   └── utils/              # Utility module tests
│   ├── integration/            # Multi-component pipeline tests
│   └── fixtures/               # Shared test data and helpers
│       ├── test_helpers.h      # Common test utilities
│       └── sample_programs/    # Test C programs
├── api/                        # Python Flask API tests
│   ├── unit/                   # API unit tests
│   ├── integration/            # API integration tests
│   └── fixtures/               # Test data
├── frontend/                   # React frontend tests
│   ├── components/             # Component tests
│   ├── integration/            # Integration tests
│   └── e2e/                    # End-to-end tests
├── system/                     # Full-stack system tests
├── CMakeLists.txt              # C++ test build configuration
└── README.md                   # This file
```

### Testing Frameworks

- **C++ Backend:** GoogleTest (gtest)
- **Python API:** pytest + requests
- **React Frontend:** Jest + React Testing Library + Vitest
- **E2E:** Playwright or Cypress (TBD)

---

## Running Tests

### C++ Backend Tests

#### Prerequisites
```bash
# Install dependencies (macOS)
brew install cmake googletest

# Or build GoogleTest from source (handled by CMakeLists.txt)
```

#### Build and Run All Tests
```bash
cd /path/to/software-engineering-project
mkdir -p build && cd build
cmake ..
make

# Run all tests with output
ctest --output-on-failure --verbose

# Or run tests directly
./test_lexer_basic
./test_lexer_keywords
./test_parser_expressions
# etc.
```

#### Run Specific Test Suites
```bash
# Lexer tests only
./test_lexer_basic
./test_lexer_keywords
./test_lexer_operators
./test_lexer_literals
./test_lexer_error_recovery

# Parser tests only
./test_parser_expressions
./test_parser_statements
./test_parser_declarations
```

#### Run Tests with GoogleTest Filters
```bash
# Run specific test case
./test_lexer_basic --gtest_filter="LexerBasicTest.EmptySourceProducesEOF"

# Run all tests matching pattern
./test_lexer_basic --gtest_filter="*Comment*"

# Run tests and output XML report
./test_lexer_basic --gtest_output=xml:test_results.xml
```

### Python API Tests (Coming Soon)
```bash
cd api
pytest tests/
pytest tests/ -v                 # Verbose output
pytest tests/ --cov=app          # With coverage
```

### Frontend Tests (Coming Soon)
```bash
cd frontend
npm test                         # Run all tests
npm test -- --coverage           # With coverage
npm run test:e2e                 # End-to-end tests
```

---

## Test Organization

### Backend Unit Tests

#### Lexer Tests (`backend/unit/lexer/`)

| File | Purpose | Coverage |
|------|---------|----------|
| `test_lexer_basic.cpp` | Basic functionality, whitespace, comments, EOF | Empty sources, position tracking |
| `test_lexer_keywords.cpp` | All 32 C89 keywords, case sensitivity | Keyword vs identifier differentiation |
| `test_lexer_operators.cpp` | All operators, precedence disambiguation | Multi-char operators, longest match |
| `test_lexer_literals.cpp` | Integer, float, char, string literals | Escape sequences, different bases |
| `test_lexer_error_recovery.cpp` | Error detection and recovery | Unterminated literals, invalid chars |

**Key Test Cases:**
- ✅ Empty source → EOF only
- ✅ All 32 C89 keywords recognized
- ✅ Operators: `++` vs `+ +` disambiguation
- ✅ Integer literals: decimal, hex (0x), octal (0)
- ✅ Float literals: scientific notation
- ✅ String/char escape sequences: `\n`, `\t`, `\x41`
- ✅ Error recovery: multiple errors in same source
- ✅ Position tracking: line, column, filename

#### Parser Tests (`backend/unit/parser/`)

| File | Purpose | Coverage |
|------|---------|----------|
| `test_parser_expressions.cpp` | Expression parsing, precedence | Binary, unary, function calls, member access |
| `test_parser_statements.cpp` | Statement parsing | If, while, for, return, compound statements |
| `test_parser_declarations.cpp` | Top-level declarations | Variables, functions, structs, pointers, arrays |

**Key Test Cases:**
- ✅ Primary expressions: literals, identifiers, `(expr)`
- ✅ Binary operators: precedence (`1 + 2 * 3` → `1 + (2 * 3)`)
- ✅ Associativity: left-to-right (`10 - 5 - 2` → `(10 - 5) - 2`)
- ✅ Unary operators: `-`, `!`, `~`, `++`, `--`
- ✅ Function calls: `foo(a, b, c)`
- ✅ Control flow: if/else, while, for
- ✅ Declarations: functions, variables, structs

### Test Helpers (`backend/fixtures/test_helpers.h`)

Common utilities available to all tests:

```cpp
// Token comparison
ASSERT_TOKEN_EQ(token, TokenType::KW_INT, "int");
ASSERT_TOKEN_TYPE(token, TokenType::IDENTIFIER);

// Lex helpers
auto tokens = lex_without_errors("int x = 5;");
auto tokens_with_err = lex_with_errors("\"unterminated", 1);

// Parse helpers
auto expr = parse_expression_without_errors("a + b");
auto program = parse_program_without_errors("int main() { return 0; }");

// AST verification
assert_identifier(expr.get(), "myVar");
assert_int_literal(expr.get(), 42);
assert_binary_op(expr.get(), "+");

// Error tracking
ErrorCapture capture(lexer.getErrorHandler());
// ... perform operations ...
EXPECT_EQ(capture.new_error_count(), 2);
```

---

## Writing New Tests

### Step 1: Choose Test Location

- **Unit test** for isolated component → `backend/unit/<module>/`
- **Integration test** for multi-component → `backend/integration/`
- **System test** for full pipeline → `system/`

### Step 2: Create Test File

Template for new test file:

```cpp
/**
 * ==============================================================================
 * [Component] [Feature] Tests
 * ==============================================================================
 *
 * Module Under Test: [ModuleName] (header.h, impl.cpp)
 *
 * Purpose:
 *   [Brief description of what this test file covers]
 *
 * Coverage:
 *   ✓ [Feature 1]
 *   ✓ [Feature 2]
 *   ✓ [Edge cases]
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "module_under_test.h"

using namespace mycc_test;

class MyFeatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup
    }

    void TearDown() override {
        // Cleanup
    }
};

TEST_F(MyFeatureTest, DescriptiveTestName) {
    // Arrange
    auto input = "test input";

    // Act
    auto result = function_under_test(input);

    // Assert
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->value, expected_value);
}
```

### Step 3: Update CMakeLists.txt

Add your test to `tests/CMakeLists.txt`:

```cmake
add_executable(test_new_feature
    backend/unit/module/test_new_feature.cpp
    ${CMAKE_SOURCE_DIR}/src/module/implementation.cpp
    # Add dependencies...
)
target_link_libraries(test_new_feature
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_new_feature)
```

### Step 4: Build and Run

```bash
cd build
cmake ..
make test_new_feature
./test_new_feature
```

---

## Testing Standards

### Naming Conventions

**Test Files:** `test_<module>_<feature>.cpp`
**Test Fixtures:** `<Module><Feature>Test`
**Test Cases:** `<Action><Scenario>` (e.g., `ParsesIntegerLiteral`, `HandlesInvalidInput`)

### AAA Pattern

Structure every test using **Arrange-Act-Assert**:

```cpp
TEST_F(MyTest, DoesExpectedBehavior) {
    // Arrange: Set up test data
    std::string input = "test input";
    int expected = 42;

    // Act: Execute the code under test
    auto result = function_under_test(input);

    // Assert: Verify outcomes
    ASSERT_EQ(result, expected);
}
```

### Assertions

- **`ASSERT_*`** → Stops test on failure (use for critical checks)
- **`EXPECT_*`** → Continues test on failure (use for multiple checks)

```cpp
ASSERT_NE(ptr, nullptr);      // Must pass
EXPECT_EQ(value, 42);         // Log failure, continue
EXPECT_TRUE(condition);
EXPECT_STREQ(str1, str2);
```

### Test Documentation

Every test should answer:
- **What** is being tested?
- **Why** is this test important?
- **How** does it verify correctness?

Use comments liberally:

```cpp
/**
 * Test: Complex operator precedence
 *
 * Verifies: Multiplication binds tighter than addition
 * Input: "1 + 2 * 3"
 * Expected AST: (1 + (2 * 3))
 */
TEST_F(ParserTest, RespectsPrecedence) { ... }
```

---

## CI/CD Integration

### GitHub Actions Workflow (TBD)

```yaml
name: CI Tests

on: [push, pull_request]

jobs:
  backend-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake g++ libgtest-dev
      - name: Build Tests
        run: |
          mkdir build && cd build
          cmake ..
          make
      - name: Run Tests
        run: cd build && ctest --output-on-failure

  api-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Set up Python
        uses: actions/setup-python@v2
        with:
          python-version: '3.9'
      - name: Install Dependencies
        run: pip install -r api/requirements.txt pytest
      - name: Run Tests
        run: cd api && pytest

  frontend-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Set up Node
        uses: actions/setup-node@v2
        with:
          node-version: '18'
      - name: Install Dependencies
        run: cd frontend && npm install
      - name: Run Tests
        run: cd frontend && npm test -- --coverage
```

---

## Coverage Reports

### Generate C++ Coverage (gcov/lcov)

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage" ..
make
ctest
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
open coverage_report/index.html
```

### Coverage Goals

- **Unit Tests:** >80% line coverage per module
- **Integration Tests:** >70% end-to-end coverage
- **Critical Paths:** 100% (lexer, parser, semantic analysis)

---

## Next Steps

### Immediate Priorities

1. ✅ Lexer unit tests (complete)
2. ✅ Parser unit tests (complete)
3. ⏳ Semantic analyzer unit tests
4. ⏳ IR generation unit tests
5. ⏳ Code generation unit tests

### Future Enhancements

- [ ] Fuzzing tests for lexer/parser
- [ ] Performance benchmarks
- [ ] Memory leak detection (Valgrind/AddressSanitizer)
- [ ] Mutation testing
- [ ] Contract testing for API

---

## Contact

For questions about testing infrastructure:
- Review test examples in `backend/unit/lexer/`
- Check `test_helpers.h` for available utilities
- Follow existing patterns for consistency

**Testing Motto:** *"If it's not tested, it's broken."*
