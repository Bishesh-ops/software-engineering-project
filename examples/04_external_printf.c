// Example 4: External Function Call (printf)
// Demonstrates calling external library functions

extern int printf(char* format, ...);

int main() {
    printf("Hello, World!\n");
    printf("The answer is: %d\n", 42);
    return 0;
}

/*
 * To compile and run:
 *
 * ./bin/compiler examples/04_external_printf.c -o output.s
 * as -o output.o output.s
 * gcc -o output output.o  # gcc links libc automatically
 * ./output
 *
 * Expected Output:
 * Hello, World!
 * The answer is: 42
 *
 * Expected Assembly Output (simplified):
 *
 *     .data
 * .LC0:
 *     .asciz "Hello, World!\n"
 * .LC1:
 *     .asciz "The answer is: %d\n"
 *
 *     .text
 *     .globl main
 * main:
 *     pushq   %rbp
 *     movq    %rsp, %rbp
 *
 *     # Call printf("Hello, World!\n")
 *     leaq    .LC0(%rip), %rdi   # Load string address
 *     xorq    %rax, %rax         # Clear %rax (no vector args)
 *     call    printf@PLT
 *
 *     # Call printf("The answer is: %d\n", 42)
 *     leaq    .LC1(%rip), %rdi   # Format string
 *     movq    $42, %rsi          # Second argument
 *     xorq    %rax, %rax
 *     call    printf@PLT
 *
 *     # return 0
 *     movq    $0, %rax
 *
 *     leave
 *     ret
 *
 * Note: System V AMD64 ABI calling convention:
 * - First 6 integer args: %rdi, %rsi, %rdx, %rcx, %r8, %r9
 * - Floating point args: %xmm0-%xmm7
 * - %rax holds number of vector registers used (0 for printf)
 * - Stack must be 16-byte aligned before call
 */
