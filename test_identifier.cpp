#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <iostream>
#include <vector>

using namespace std;

int main()
{
    string source = "myVariable";
    
    Lexer lexer(source, "test.c");
    vector<Token> tokens = lexer.lexAll();
    
    if (tokens.empty() || tokens[0].type != TokenType::IDENTIFIER)
    {
        cout << "ERROR: Expected IDENTIFIER token, got something else" << endl;
        return 1;
    }
    
    cout << "Token created: " << tokens[0].to_string() << endl;
    
    Parser parser(tokens);
    auto node = parseIdentifier(parser);
    
    if (!node)
    {
        cout << "ERROR: parseIdentifier returned nullptr" << endl;
        return 1;
    }
    
    if (node->getNodeType() != ASTNodeType::IDENTIFIER_EXPR)
    {
        cout << "ERROR: Expected IDENTIFIER_EXPR node type" << endl;
        return 1;
    }
    
    IdentifierNode *idNode = dynamic_cast<IdentifierNode*>(node.get());
    if (!idNode)
    {
        cout << "ERROR: Could not cast to IdentifierNode" << endl;
        return 1;
    }
    
    cout << "SUCCESS!" << endl;
    cout << "  Variable name: " << idNode->getName() << endl;
    cout << "  Location: " << idNode->getLocation().toString() << endl;
    
    return 0;
}

