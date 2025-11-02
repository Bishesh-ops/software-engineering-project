#include "parser.h"
#include <string>

std::unique_ptr<ASTNode> parseIdentifier(Parser &parser)
{
    if (!parser.check(TokenType::IDENTIFIER))
    {
        return nullptr;
    }

    Token token = parser.advance();
    std::string name = std::string(token.value);
    SourceLocation location(token.filename, token.line, token.column);

    return std::make_unique<IdentifierNode>(name, location);
}

