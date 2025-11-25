# Constant Folding Optimization

## Overview

This document describes the implementation of the **Constant Folding** optimization pass for the C compiler's intermediate representation (IR).

## User Story

**As a compiler**
**I want to** evaluate constant expressions at compile time
**So that** runtime computation is reduced

### Acceptance Criteria

- ✅ `2 + 3` → `5`
- ✅ `10 * 0` → `0`
- ✅ Works on IR level

## Implementation

### Files Created

1. **`include/ir_optimizer.h`** - Header file defining the IROptimizer class
2. **`src/ir/ir_optimizer.cpp`** - Implementation of optimization passes
3. **`tests/test_constant_folding.cpp`** - Comprehensive test suite with 12 test cases
4. **`examples/constant_folding_example.cpp`** - Educational examples demonstrating the optimization

### Architecture

The constant folding optimization is implemented as part of the `IROptimizer` class, which is designed to support multiple optimization passes:

```cpp
class IROptimizer {
public:
    // Optimization Passes
    int constantFoldingPass(IRFunction* function);
    int deadCodeEliminationPass(IRFunction* function);      // Future
    int commonSubexpressionEliminationPass(IRFunction* function);  // Future

    // Optimization Pipeline
    void optimize(IRFunction* function);
    void optimize(std::vector<std::unique_ptr<IRFunction>>& functions);
};
```

### Algorithm

The constant folding pass works as follows:

1. **Traverse all basic blocks** in the function
2. **Identify arithmetic instructions** with both operands as constants
3. **Evaluate the operation** at compile time
4. **Replace the instruction** with a MOVE instruction containing the result
5. **Track statistics** for reporting

#### Supported Operations

- **ADD**: `a + b` → computed sum
- **SUB**: `a - b` → computed difference
- **MUL**: `a * b` → computed product
- **DIV**: `a / b` → computed quotient (with division-by-zero protection)
- **MOD**: `a % b` → computed remainder (with modulo-by-zero protection)

#### Safety Features

- **Division by zero**: Not optimized, preserved for runtime error handling
- **Modulo by zero**: Not optimized, preserved for runtime error handling
- **Type safety**: Only operates on integer constants
- **SSA preservation**: Maintains SSA form correctness

### Examples

#### Example 1: Simple Addition
```
Before:  t0 = add 2, 3
After:   t0 = move 5
```

#### Example 2: Multiplication by Zero
```
Before:  t0 = mul 10, 0
After:   t0 = move 0
```

#### Example 3: Mixed Operations
```
Before:
  t0 = add 2, 3      // Constants - will optimize
  t1 = add x, 5      // Variable - won't optimize
  t2 = mul 4, 7      // Constants - will optimize

After:
  t0 = move 5        // Optimized
  t1 = add x, 5      // Unchanged
  t2 = move 28       // Optimized
```

#### Example 4: Control Flow
```
Before:
  entry:
    jump_if_false cond, else_block
  then_block:
    x1 = add 100, 200
    jump merge
  else_block:
    x2 = mul 50, 2
    jump merge

After:
  entry:
    jump_if_false cond, else_block
  then_block:
    x1 = move 300      // Optimized
    jump merge
  else_block:
    x2 = move 100      // Optimized
    jump merge
```

## Testing

### Test Suite

The test suite includes 12 comprehensive test cases:

1. **Simple Addition** - Basic `2 + 3 → 5`
2. **Multiplication by Zero** - `10 * 0 → 0`
3. **Subtraction** - `15 - 7 → 8`
4. **Division** - `20 / 4 → 5`
5. **Modulo** - `17 % 5 → 2`
6. **Multiple Operations** - Multiple constant operations in one block
7. **Mixed Operations** - Constants and variables mixed
8. **Division by Zero** - Safety check (should NOT optimize)
9. **Modulo by Zero** - Safety check (should NOT optimize)
10. **Negative Numbers** - Handling negative constants
11. **Multiple Basic Blocks** - Optimization across control flow
12. **Optimization Statistics** - Tracking and reporting

### Running Tests

```bash
# Build and run constant folding tests
make test_constant_folding

# Run all tests including constant folding
make test

# Build the example
make bin/constant_folding_example.exe

# Run the example
./bin/constant_folding_example.exe
```

### Test Results

All 12 tests pass successfully:

```
========================================
All Constant Folding Tests Complete!
========================================
```

## Performance Impact

### Compile-Time Impact

- **Minimal overhead**: Single pass through IR instructions
- **O(n) complexity**: Linear in number of instructions
- **Fast evaluation**: Simple integer arithmetic

### Runtime Benefits

- **Reduced instructions**: Arithmetic operations replaced with moves
- **Faster execution**: No runtime computation for constant expressions
- **Smaller code**: Fewer instructions in generated code

### Optimization Statistics

The optimizer tracks and reports statistics:

```
========================================
IR Optimization Report
========================================
Constant Folding:              15 optimizations
Dead Code Elimination:         0 optimizations
Common Subexpression Elim.:    0 optimizations
----------------------------------------
Total Optimizations:           15
========================================
```

## Integration

### Using the Optimizer

```cpp
#include "ir_optimizer.h"

// Create optimizer
IROptimizer optimizer;

// Optimize a single function
optimizer.constantFoldingPass(&function);

// Or run full optimization pipeline
optimizer.optimize(&function);

// Get statistics
int count = optimizer.getConstantFoldingCount();
optimizer.printOptimizationReport();
```

### Integration with Compilation Pipeline

The optimizer fits into the compilation pipeline as follows:

```
Lexer → Parser → Semantic Analyzer → IR Generator → [OPTIMIZER] → Code Generator
```

Current usage in integration tests (optional):
```cpp
// After IR generation
IROptimizer optimizer;
optimizer.optimize(&irFunction);
```

## Design Decisions

### 1. Instruction Replacement Strategy

**Decision**: Replace arithmetic instructions with MOVE instructions rather than propagating constants.

**Rationale**:
- Maintains SSA form integrity
- Simplifies implementation
- Enables future dead code elimination (unused MOVEs can be removed)
- Clear audit trail of optimizations

### 2. Safety-First Approach

**Decision**: Do not optimize division/modulo by zero.

**Rationale**:
- Preserve runtime error behavior
- Allow proper error handling
- Avoid changing program semantics
- Maintain debuggability

### 3. Type Limitations

**Decision**: Only optimize integer constant operations.

**Rationale**:
- Phase 1 implementation focus
- Integer arithmetic is deterministic
- Floating-point requires careful handling of precision
- Future enhancement opportunity

### 4. SSA-Aware Design

**Decision**: Work directly with SSA form, preserving SSA properties.

**Rationale**:
- Already in SSA form from IR generation
- Easier to implement and verify correctness
- Enables future optimizations that depend on SSA

## Future Enhancements

### Phase 2: Additional Optimizations

1. **Dead Code Elimination**
   - Remove unused MOVE instructions
   - Eliminate unreachable code

2. **Common Subexpression Elimination (CSE)**
   - Identify repeated computations
   - Reuse previously computed values

3. **Constant Propagation**
   - Track constant values through the program
   - Enable more constant folding opportunities

### Phase 3: Advanced Features

1. **Floating-Point Support**
   - Add support for float/double constant folding
   - Handle precision and rounding correctly

2. **Algebraic Simplifications**
   - `x * 1` → `x`
   - `x + 0` → `x`
   - `x * 0` → `0` (even with variables)

3. **Strength Reduction**
   - `x * 2` → `x << 1`
   - `x / 2` → `x >> 1`

## Coding Standards

The implementation follows the project's existing coding conventions:

- **C++17 standard**
- **Class-based design** with clear separation of concerns
- **Comprehensive documentation** with clear comments
- **Consistent naming**: camelCase for methods, PascalCase for classes
- **RAII principles** with smart pointers where appropriate
- **const correctness** throughout

## Testing Standards

All tests follow the project's testing patterns:

- **Clear test names** describing what is being tested
- **Before/After visualization** for easy understanding
- **Assertion-based verification**
- **Comprehensive edge case coverage**
- **Pass/Fail reporting** with descriptive messages

## Documentation Standards

- **User story-driven** design and implementation
- **Acceptance criteria** clearly defined and met
- **Examples** for educational purposes
- **Clear reasoning** for design decisions
- **Future enhancement** roadmap

## Summary

The constant folding optimization has been successfully implemented and integrated into the C compiler project. It:

- ✅ Meets all acceptance criteria
- ✅ Works at IR level with SSA form
- ✅ Handles all arithmetic operations
- ✅ Includes comprehensive test coverage (12 tests, all passing)
- ✅ Provides educational examples
- ✅ Includes optimization statistics
- ✅ Maintains code quality and standards
- ✅ Preserves program semantics and safety
- ✅ Integrates cleanly with existing codebase
- ✅ Provides foundation for future optimizations

## References

- **IR Header**: `include/ir.h` (lines 1-547)
- **IR Implementation**: `src/ir/ir.cpp` (lines 1-223)
- **Optimizer Header**: `include/ir_optimizer.h` (lines 1-86)
- **Optimizer Implementation**: `src/ir/ir_optimizer.cpp` (lines 1-234)
- **Test Suite**: `tests/test_constant_folding.cpp` (lines 1-end)
- **Example Code**: `examples/constant_folding_example.cpp` (lines 1-end)
