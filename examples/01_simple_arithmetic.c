// Example 1: Simple Arithmetic
// Demonstrates basic integer arithmetic operations

int main() {
    int x = 10;
    int y = 20;
    int sum = x + y;
    return sum;  // Returns 30
}

/*
 * To compile and run:
 *
 * ./bin/compiler examples/01_simple_arithmetic.c -o output.s
 * as -o output.o output.s
 * gcc -o output output.o
 * ./output
 * echo $?  # Should print: 30
 *
 * Expected Assembly Output (simplified):
 *
 *     .text
 *     .globl main
 * main:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *     subq    $24, %rsp
 *
 *     # int x = 10
 *     movq    $10, -8(%rbp)
 *
 *     # int y = 20
 *     movq    $20, -16(%rbp)
 *
 *     # int sum = x + y
 *     movq    -8(%rbp), %rax
 *     addq    -16(%rbp), %rax
 *     movq    %rax, -24(%rbp)
 *
 *     # return sum
 *     movq    -24(%rbp), %rax
 *
 *     leave
 *     ret
 */
