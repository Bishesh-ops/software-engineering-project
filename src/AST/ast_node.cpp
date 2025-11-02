#include "ast.h"
#include <memory>

ASTNode::ASTNode(ASTNodeType type, const SourceLocation &location)
    : node_type_(type), location_(location) {}

