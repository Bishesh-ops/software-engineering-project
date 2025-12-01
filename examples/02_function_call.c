// Example 2: Function Call
// Demonstrates function definition, calls, and parameter passing

int add(int a, int b) {
    return a + b;
}

int multiply(int x, int y) {
    return x * y;
}

int main() {
    int sum = add(10, 5);          // sum = 15
    int product = multiply(6, 7);  // product = 42
    return product;                // Returns 42
}

/*
 * To compile and run:
 *
 * ./bin/compiler examples/02_function_call.c -o output.s
 * as -o output.o output.s
 * gcc -o output output.o
 * ./output
 * echo $?  # Should print: 42
 *
 * Expected Assembly Output (simplified):
 *
 *     .text
 *     .globl add
 * add:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *
 *     # Parameters: a in %rdi, b in %rsi (System V ABI)
 *     movq    %rdi, -8(%rbp)
 *     movq    %rsi, -16(%rbp)
 *
 *     # return a + b
 *     movq    -8(%rbp), %rax
 *     addq    -16(%rbp), %rax
 *
 *     leave
 *     ret
 *
 *     .globl multiply
 * multiply:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *
 *     movq    %rdi, -8(%rbp)
 *     movq    %rsi, -16(%rbp)
 *
 *     # return x * y
 *     movq    -8(%rbp), %rax
 *     imulq   -16(%rbp), %rax
 *
 *     leave
 *     ret
 *
 *     .globl main
 * main:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *     subq    $16, %rsp
 *
 *     # Call add(10, 5)
 *     movq    $10, %rdi
 *     movq    $5, %rsi
 *     call    add
 *     movq    %rax, -8(%rbp)
 *
 *     # Call multiply(6, 7)
 *     movq    $6, %rdi
 *     movq    $7, %rsi
 *     call    multiply
 *     movq    %rax, -16(%rbp)
 *
 *     # return product
 *     movq    -16(%rbp), %rax
 *
 *     leave
 *     ret
 */
