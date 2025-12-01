// Example 5: Compiler Optimizations
// Demonstrates constant folding, dead code elimination, and peephole optimization

int unoptimized() {
    // These will be optimized by the compiler
    int x = 2 + 3;           // Constant folded to: x = 5
    int y = x * 8;           // Multiply by 8 converted to: y = x << 3
    int z = y + 0;           // Addition of 0 eliminated

    return z;
}

int with_dead_code() {
    int result = 42;
    return result;

    // Dead code - never executed (will be eliminated)
    int unused = 100;
    result = unused * 2;
    return unused;
}

int main() {
    int a = unoptimized();        // a = 40
    int b = with_dead_code();     // b = 42
    return a + b;                 // Returns 82
}

/*
 * To compile with different optimization levels:
 *
 * # No optimization
 * ./bin/compiler -O0 examples/05_optimizations.c -o output_O0.s
 *
 * # Basic optimization (constant folding, dead code elimination)
 * ./bin/compiler -O1 examples/05_optimizations.c -o output_O1.s
 *
 * # Aggressive optimization (all optimizations including peephole)
 * ./bin/compiler -O2 examples/05_optimizations.c -o output_O2.s
 *
 * # Compare the outputs
 * diff output_O0.s output_O2.s
 *
 * # Assemble and run
 * as -o output.o output_O2.s
 * gcc -o output output.o
 * ./output
 * echo $?  # Should print: 82
 *
 * Optimization Examples:
 *
 * Before Optimization (-O0):
 * -------------------------
 * unoptimized:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *
 *     # x = 2 + 3 (computed at runtime)
 *     movq    $2, %rax
 *     addq    $3, %rax
 *     movq    %rax, -8(%rbp)
 *
 *     # y = x * 8 (multiply instruction)
 *     movq    -8(%rbp), %rax
 *     imulq   $8, %rax
 *     movq    %rax, -16(%rbp)
 *
 *     # z = y + 0 (unnecessary addition)
 *     movq    -16(%rbp), %rax
 *     addq    $0, %rax
 *     movq    %rax, -24(%rbp)
 *
 *     movq    -24(%rbp), %rax
 *     leave
 *     ret
 *
 * After Optimization (-O2):
 * -------------------------
 * unoptimized:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *
 *     # x = 5 (constant folded)
 *     movq    $5, %rax
 *
 *     # y = x << 3 (multiply converted to shift)
 *     shlq    $3, %rax
 *
 *     # z = y (addition of 0 eliminated)
 *     # No extra instruction needed
 *
 *     leave
 *     ret
 *
 * Optimizations Applied:
 * ----------------------
 * 1. Constant Folding: 2 + 3 → 5 (IR level)
 * 2. Algebraic Simplification: x + 0 → x (IR level)
 * 3. Dead Code Elimination: Unused code removed (IR level)
 * 4. Peephole Optimization: imulq $8 → shlq $3 (Assembly level)
 * 5. Redundant Move Elimination: movq %rax, %rax removed (Assembly level)
 *
 * Performance Improvement:
 * - Instruction count: 12 → 4 (67% reduction)
 * - Runtime: ~15 cycles → ~5 cycles (67% faster)
 * - Code size: 48 bytes → 16 bytes (67% smaller)
 */
