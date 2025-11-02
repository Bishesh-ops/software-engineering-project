#include "ast.h"
#include <string>

IdentifierNode::IdentifierNode(const std::string &name, const SourceLocation &location)
    : ASTNode(ASTNodeType::IDENTIFIER_EXPR, location), name_(name) {}

void IdentifierNode::accept(ASTVisitor *visitor)
{
}

