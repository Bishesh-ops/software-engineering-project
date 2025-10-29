/**
 * @file test_lexer.cpp
 * @brief A simple driver program to test the Lexer component.
 *
 * This program instantiates the Lexer with a comprehensive "kitchen sink"
 * test string, calls lexAll() to tokenize it, and prints each token
 * to the console for verification.
 */

#include "lexer.h"
#include <iostream>

using namespace std;

int main()
{
    // --- Comprehensive "Kitchen Sink" Test Case ---
    // This string tests every major feature of the lexer:
    // - Preprocessor tokens (#, ##)
    // - Keywords (int, char, void, if, return)
    // - Identifiers (_my_var, main)
    // - Delimiters ((), {}, ;)
    // - Operators (->, =, +, *, ==, !=, ?:)
    // - String Literals (with escapes)
    // - Char Literals (valid, escape)
    // - Number Literals (decimal, hex, octal, float, suffixes)
    // - Errors (unterminated string, invalid char, unknown symbol)
    // - Multi-line tracking

    const string test_source = R"(
#define MY_MACRO(a, b) a ## b

int main(void) {
    char* s = "Hello\t\"World\"!";
    char c = '\n';
    
    struct S { int m; } s_inst;
    s_inst.m = 10;
    
    int x = 42;          // Decimal
    int h = 0x1aF;       // Hex
    int o = 077;         // Octal
    float f = 1.23f;
    double d = .5e-10;
    
    unsigned long ul = 100UL;
    long long llu = 200LLU;
    
    if (x == h && o != 0) {
        return (x > o) ? 1 : 0;
    }
    
    // Lexical Errors
    'abc' '' @
    "unterminated string
)";

    // --- Lexer Execution ---
    cout << "--- Tokenizing Source ---" << endl;
    cout << test_source << endl;
    cout << "-------------------------" << endl;

    Lexer lexer(test_source);
    auto tokens = lexer.lexAll();

    // --- Print Results ---
    for (const auto &token : tokens)
    {
        cout << token.to_string() << endl;
    }

    return 0;
}