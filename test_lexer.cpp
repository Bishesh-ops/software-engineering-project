#include "lexer.h"
#include <iostream>

using namespace std;

int main()
{
    // Lexer lexer("int main if myVariable integer elseif float Bjse _jeoi");
    Lexer lexer("'a' 'Z' '\\n' '\\t' '\\\\' '\\'' '' 'abc' 'x");
    //Lexer lexer("int x = 42;")
    auto tokens = lexer.lexAll();

    for (const auto& token : tokens)
    {
        cout << token.to_string() << endl;
    }
    return 0;
}