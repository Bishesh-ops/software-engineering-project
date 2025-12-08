#include "json_serializer.h"
#include <sstream>
#include <iomanip>

// ============================================================================
// Utility Methods
// ============================================================================

std::string JSONSerializer::escapeJSON(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

std::string JSONSerializer::tokenTypeToString(TokenType type) {
    // Map token types to strings
    switch (type) {
        case TokenType::KW_INT: return "KW_INT";
        case TokenType::KW_FLOAT: return "KW_FLOAT";
        case TokenType::KW_CHAR: return "KW_CHAR";
        case TokenType::KW_VOID: return "KW_VOID";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_FOR: return "KW_FOR";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::KW_STRUCT: return "KW_STRUCT";

        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT_LITERAL: return "INT_LITERAL";
        case TokenType::FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TokenType::CHAR_LITERAL: return "CHAR_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";

        case TokenType::OP_PLUS: return "OP_PLUS";
        case TokenType::OP_MINUS: return "OP_MINUS";
        case TokenType::OP_STAR: return "OP_STAR";
        case TokenType::OP_SLASH: return "OP_SLASH";
        case TokenType::OP_ASSIGN: return "OP_ASSIGN";
        case TokenType::OP_EQ: return "OP_EQ";
        case TokenType::OP_NE: return "OP_NE";
        case TokenType::OP_LT: return "OP_LT";
        case TokenType::OP_GT: return "OP_GT";
        case TokenType::OP_LE: return "OP_LE";
        case TokenType::OP_GE: return "OP_GE";

        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";

        case TokenType::EOF_TOKEN: return "EOF_TOKEN";
        case TokenType::UNKNOWN: return "UNKNOWN";

        default: return "UNKNOWN";
    }
}

std::string JSONSerializer::nodeTypeToString(ASTNodeType type) {
    switch (type) {
        case ASTNodeType::BINARY_EXPR: return "BinaryExpr";
        case ASTNodeType::UNARY_EXPR: return "UnaryExpr";
        case ASTNodeType::LITERAL_EXPR: return "LiteralExpr";
        case ASTNodeType::IDENTIFIER_EXPR: return "IdentifierExpr";
        case ASTNodeType::CALL_EXPR: return "CallExpr";
        case ASTNodeType::ASSIGNMENT_EXPR: return "AssignmentExpr";
        case ASTNodeType::ARRAY_ACCESS_EXPR: return "ArrayAccessExpr";

        case ASTNodeType::IF_STMT: return "IfStmt";
        case ASTNodeType::WHILE_STMT: return "WhileStmt";
        case ASTNodeType::FOR_STMT: return "ForStmt";
        case ASTNodeType::RETURN_STMT: return "ReturnStmt";
        case ASTNodeType::COMPOUND_STMT: return "CompoundStmt";
        case ASTNodeType::EXPRESSION_STMT: return "ExpressionStmt";

        case ASTNodeType::VAR_DECL: return "VarDecl";
        case ASTNodeType::FUNCTION_DECL: return "FunctionDecl";
        case ASTNodeType::STRUCT_DECL: return "StructDecl";

        default: return "Unknown";
    }
}

std::string JSONSerializer::jsonObject(const std::vector<std::pair<std::string, std::string>>& fields) {
    std::ostringstream oss;
    oss << "{";
    for (size_t i = 0; i < fields.size(); i++) {
        oss << "\"" << fields[i].first << "\":" << fields[i].second;
        if (i < fields.size() - 1) {
            oss << ",";
        }
    }
    oss << "}";
    return oss.str();
}

std::string JSONSerializer::jsonArray(const std::vector<std::string>& elements) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elements.size(); i++) {
        oss << elements[i];
        if (i < elements.size() - 1) {
            oss << ",";
        }
    }
    oss << "]";
    return oss.str();
}

// ============================================================================
// Token Serialization
// ============================================================================

std::string JSONSerializer::serializeToken(const Token& token) {
    std::vector<std::pair<std::string, std::string>> fields;

    fields.push_back({"type", "\"" + tokenTypeToString(token.type) + "\""});
    fields.push_back({"value", "\"" + escapeJSON(std::string(token.value)) + "\""});
    fields.push_back({"line", std::to_string(token.line)});
    fields.push_back({"column", std::to_string(token.column)});
    fields.push_back({"filename", "\"" + escapeJSON(token.filename) + "\""});

    return jsonObject(fields);
}

std::string JSONSerializer::serializeTokens(const std::vector<Token>& tokens) {
    std::vector<std::string> tokenJSON;
    for (const auto& token : tokens) {
        tokenJSON.push_back(serializeToken(token));
    }
    return jsonArray(tokenJSON);
}

// ============================================================================
// Hex Dump Serialization
// ============================================================================

std::string JSONSerializer::hexDump(const void* data, size_t size) {
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    std::ostringstream oss;

    std::vector<std::pair<std::string, std::string>> fields;

    // Hex representation
    std::ostringstream hexStream;
    hexStream << "\"";
    for (size_t i = 0; i < size; i++) {
        hexStream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
        if (i < size - 1) {
            hexStream << " ";
        }
    }
    hexStream << "\"";

    // ASCII representation
    std::ostringstream asciiStream;
    asciiStream << "\"";
    for (size_t i = 0; i < size; i++) {
        char c = bytes[i];
        if (c >= 32 && c <= 126) {
            asciiStream << c;
        } else {
            asciiStream << ".";
        }
    }
    asciiStream << "\"";

    fields.push_back({"size", std::to_string(size)});
    fields.push_back({"hex", hexStream.str()});
    fields.push_back({"ascii", asciiStream.str()});

    return jsonObject(fields);
}

std::string JSONSerializer::hexDumpString(const std::string& str) {
    return hexDump(str.data(), str.size());
}

// ============================================================================
// AST Serialization - Expression
// ============================================================================

std::string JSONSerializer::serializeLiteralExpr(const LiteralExpr* expr) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"LiteralExpr\""});
    fields.push_back({"value", "\"" + escapeJSON(expr->getValue()) + "\""});
    fields.push_back({"line", std::to_string(expr->getLine())});
    fields.push_back({"column", std::to_string(expr->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeIdentifierExpr(const IdentifierExpr* expr) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"IdentifierExpr\""});
    fields.push_back({"name", "\"" + escapeJSON(expr->getName()) + "\""});
    fields.push_back({"line", std::to_string(expr->getLine())});
    fields.push_back({"column", std::to_string(expr->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeBinaryExpr(const BinaryExpr* expr) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"BinaryExpr\""});
    fields.push_back({"operator", "\"" + escapeJSON(expr->getOperator()) + "\""});
    fields.push_back({"left", serializeExpression(expr->getLeft())});
    fields.push_back({"right", serializeExpression(expr->getRight())});
    fields.push_back({"line", std::to_string(expr->getLine())});
    fields.push_back({"column", std::to_string(expr->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeUnaryExpr(const UnaryExpr* expr) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"UnaryExpr\""});
    fields.push_back({"operator", "\"" + escapeJSON(expr->getOperator()) + "\""});
    fields.push_back({"operand", serializeExpression(expr->getOperand())});
    fields.push_back({"line", std::to_string(expr->getLine())});
    fields.push_back({"column", std::to_string(expr->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeCallExpr(const CallExpr* expr) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"CallExpr\""});
    fields.push_back({"callee", serializeExpression(expr->getCallee())});

    std::vector<std::string> argsJSON;
    for (const auto& arg : expr->getArguments()) {
        argsJSON.push_back(serializeExpression(arg.get()));
    }
    fields.push_back({"arguments", jsonArray(argsJSON)});
    fields.push_back({"line", std::to_string(expr->getLine())});
    fields.push_back({"column", std::to_string(expr->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeExpression(const Expression* expr) {
    if (!expr) {
        return "null";
    }

    switch (expr->getNodeType()) {
        case ASTNodeType::LITERAL_EXPR:
            return serializeLiteralExpr(static_cast<const LiteralExpr*>(expr));
        case ASTNodeType::IDENTIFIER_EXPR:
            return serializeIdentifierExpr(static_cast<const IdentifierExpr*>(expr));
        case ASTNodeType::BINARY_EXPR:
            return serializeBinaryExpr(static_cast<const BinaryExpr*>(expr));
        case ASTNodeType::UNARY_EXPR:
            return serializeUnaryExpr(static_cast<const UnaryExpr*>(expr));
        case ASTNodeType::CALL_EXPR:
            return serializeCallExpr(static_cast<const CallExpr*>(expr));
        default:
            return "{\"nodeType\":\"Unknown\"}";
    }
}

// ============================================================================
// AST Serialization - Statement
// ============================================================================

std::string JSONSerializer::serializeReturnStmt(const ReturnStmt* stmt) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"ReturnStmt\""});
    fields.push_back({"value", serializeExpression(stmt->getReturnValue())});
    fields.push_back({"line", std::to_string(stmt->getLine())});
    fields.push_back({"column", std::to_string(stmt->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeStatement(const Statement* stmt) {
    if (!stmt) {
        return "null";
    }

    switch (stmt->getNodeType()) {
        case ASTNodeType::RETURN_STMT:
            return serializeReturnStmt(static_cast<const ReturnStmt*>(stmt));
        case ASTNodeType::EXPRESSION_STMT:
            // Expression statement contains an expression
            return "{\"nodeType\":\"ExpressionStmt\"}";
        default:
            return "{\"nodeType\":\"Unknown\"}";
    }
}

// ============================================================================
// AST Serialization - Declaration
// ============================================================================

std::string JSONSerializer::serializeVarDecl(const VarDecl* decl) {
    std::vector<std::pair<std::string, std::string>> fields;
    fields.push_back({"nodeType", "\"VarDecl\""});
    fields.push_back({"name", "\"" + escapeJSON(decl->getName()) + "\""});
    fields.push_back({"type", "\"" + escapeJSON(decl->getType()) + "\""});
    fields.push_back({"line", std::to_string(decl->getLine())});
    fields.push_back({"column", std::to_string(decl->getColumn())});
    return jsonObject(fields);
}

std::string JSONSerializer::serializeDeclaration(const Declaration* decl) {
    if (!decl) {
        return "null";
    }

    switch (decl->getNodeType()) {
        case ASTNodeType::VAR_DECL:
            return serializeVarDecl(static_cast<const VarDecl*>(decl));
        default:
            return "{\"nodeType\":\"Unknown\"}";
    }
}

std::string JSONSerializer::serializeAST(const ASTNode* node) {
    if (!node) {
        return "null";
    }

    // Try as expression
    if (const Expression* expr = dynamic_cast<const Expression*>(node)) {
        return serializeExpression(expr);
    }

    // Try as statement
    if (const Statement* stmt = dynamic_cast<const Statement*>(node)) {
        return serializeStatement(stmt);
    }

    // Try as declaration
    if (const Declaration* decl = dynamic_cast<const Declaration*>(node)) {
        return serializeDeclaration(decl);
    }

    return "{\"nodeType\":\"Unknown\"}";
}
