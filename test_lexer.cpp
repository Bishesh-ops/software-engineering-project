#include "lexer.h"
#include <iostream>

using namespace std;

int main()
{
    // Lexer lexer("int main if myVariable integer elseif float Bjse _jeoi");
    // Lexer lexer("'a' 'Z' '\\n' '\\t' '\\\\' '\\'' '' 'abc' 'x'");
    //Lexer lexer("int x = 42;")

    // Lexer lexer("\"hello\" \"line1\\nline2\" \"unterminated");
    // Lexer lexer("+ - * / % < > ! ~ & | ^ = ? : ( ) { } [ ] ; : , .");

    // Test: Comments (single-line, multi-line, multi-line with newlines, nested structure, division vs comment, unterminated)
    Lexer lexer("int x; // single-line comment\nint y; /* multi-line */ int z; /* spans\nmultiple\nlines */ int a; /* /* nested */ comment */ int b; x / y; // division then comment\nint c; /* unterminated");

    auto tokens = lexer.lexAll();

    for (const auto& token : tokens)
    {
        cout << token.to_string() << endl;
    }
    return 0;
}