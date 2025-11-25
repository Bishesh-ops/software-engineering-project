/**
 * @file test_lexer.cpp
 * @brief A simple driver program to test the Lexer component.
 *
 * This program instantiates the Lexer with a longer, more complex
 * test string, calls lexAll() to tokenize it, and prints each token
 * to the console for verification and timing.
 */

#include "lexer.h"
#include <iostream>

using namespace std;

int main()
{
    // --- Longer Test Case with Edge Cases ---
    const string test_source = R"(
/* Multi-line comment test
   Starts here...
   * With asterisk inside *
   ... and ends here. */

#define ADD(x, y) ((x)+(y)) // Simple macro def

#line 100 "generated_code.h"

// Test function
int calculate(int a, int b) {
    int result = 0;
    result += ADD(a, b); // Use macro
    result *= 2;
    result--;
    ++result;

    float approx = 1.f;
    double precise = .0005e+5L; // Suffix might be ignored or cause error depending on strictness
    long hex_val = 0xDeadBeef;
    unsigned oct_val = 0123u;

    if (result > 10 && hex_val != 0) {
        char decision = (oct_val <= 100) ? 'Y' : 'N';
        if (decision == '\'') return -1; // Char literal check
    } else {
        // Pointer arithmetic and struct access
        struct Point { int x, y; };
        struct Point p = {1, 2};
        struct Point *ptr = &p;
        ptr->x = ptr->y << 1;
    }

    /* Nested attempt: /* This won't nest */ Still in outer comment */

    // Number edge cases and errors
    int bad_oct = 08; // Invalid octal
    int bad_hex = 0x;  // Invalid hex
    float bad_float = 1.2.3; // Invalid float
    float bad_exp = 1e; // Invalid exponent
    long bad_suffix = 100LGa; // Invalid suffix

    #line 5 "original.c"
    // Back to original context

    char* message = "String with \\\"escapes\\\" and\na newline.";

    // Invalid characters and recovery test
    int test = 5; @ $ % // Should produce UNKNOWNs
    test = test + /* comment mid-expression */ 3;
    'unterminated
    "unterminated 2

    // Force error count limit? (Repeat errors)
    @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @ @

    return result; // Final line
}
)";

    // --- Lexer Execution ---
    cout << "--- Tokenizing Source ---" << endl;
    cout << test_source << endl;
    cout << "-------------------------" << endl;

    // Pass an initial filename to the constructor
    Lexer lexer(test_source, "original.c");
    auto tokens = lexer.lexAll();

    // --- Print Results ---
    for (const auto& token : tokens)
    {
        cout << token.to_string() << endl;
    }

    return 0;
}