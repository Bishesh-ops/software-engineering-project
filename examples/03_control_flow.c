// Example 3: Control Flow
// Demonstrates if/else statements and recursion

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int result = factorial(5);  // 5! = 120
    return result % 100;        // Returns 20 (last two digits)
}

/*
 * To compile and run:
 *
 * ./bin/compiler examples/03_control_flow.c -o output.s
 * as -o output.o output.s
 * gcc -o output output.o
 * ./output
 * echo $?  # Should print: 20
 *
 * Expected Assembly Output (simplified):
 *
 *     .text
 *     .globl factorial
 * factorial:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *     subq    $16, %rsp
 *
 *     # Store parameter n
 *     movq    %rdi, -8(%rbp)
 *
 *     # if (n <= 1)
 *     movq    -8(%rbp), %rax
 *     cmpq    $1, %rax
 *     jg      .L_else
 *
 * .L_then:
 *     # return 1
 *     movq    $1, %rax
 *     jmp     .L_end
 *
 * .L_else:
 *     # return n * factorial(n - 1)
 *     movq    -8(%rbp), %rax
 *     subq    $1, %rax
 *     movq    %rax, %rdi
 *     call    factorial          # Recursive call
 *     movq    -8(%rbp), %rcx
 *     imulq   %rcx, %rax
 *
 * .L_end:
 *     leave
 *     ret
 *
 *     .globl main
 * main:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *     subq    $16, %rsp
 *
 *     # Call factorial(5)
 *     movq    $5, %rdi
 *     call    factorial
 *     movq    %rax, -8(%rbp)
 *
 *     # return result % 100
 *     movq    -8(%rbp), %rax
 *     cqto
 *     movq    $100, %rcx
 *     idivq   %rcx
 *     movq    %rdx, %rax         # Remainder in %rdx
 *
 *     leave
 *     ret
 */
