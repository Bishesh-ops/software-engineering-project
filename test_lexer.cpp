#include "lexer.h"
#include <iostream>

int main()
{
    Lexer lexer("int main if myVariable integer");
    // Lexer lexer("int x = 42;");
    auto tokens = lexer.lexAll();

    for (const auto& token : tokens) 
    {
        std::cout << token.to_string() << std::endl;
    }
    return 0;
}