#include "lexer.h"
#include <iostream>

using namespace std;

int main()
{
    //Lexer lexer("int main if myVariable str char < > <= >= integer elseif float Bjse _jeoi");
     Lexer lexer("int x = 0.921;");
    auto tokens = lexer.lexAll();

    for (const auto& token : tokens) 
    {
        cout << token.to_string() << endl;
    }
    return 0;
}