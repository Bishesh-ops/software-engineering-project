#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "ast.h"
#include "lexer.h"
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>

// ============================================================================
// JSON Serializer - Converts compiler structures to JSON format
// ============================================================================
// Provides JSON serialization for:
// - Tokens (from lexer)
// - AST nodes (from parser)
// - Hex dumps (for binary data)
//
// Output format follows standard JSON schema for easy parsing and validation
// ============================================================================

class JSONSerializer {
public:
    // ========================================================================
    // Token Serialization
    // ========================================================================

    // Serialize a single token to JSON
    static std::string serializeToken(const Token& token);

    // Serialize a list of tokens to JSON array
    static std::string serializeTokens(const std::vector<Token>& tokens);

    // ========================================================================
    // AST Serialization
    // ========================================================================

    // Serialize an AST node to JSON
    static std::string serializeAST(const ASTNode* node);

    // Serialize an expression to JSON
    static std::string serializeExpression(const Expression* expr);

    // Serialize a statement to JSON
    static std::string serializeStatement(const Statement* stmt);

    // Serialize a declaration to JSON
    static std::string serializeDeclaration(const Declaration* decl);

    // ========================================================================
    // Hex Dump Serialization
    // ========================================================================

    // Create hex dump of binary data in JSON format
    static std::string hexDump(const void* data, size_t size);

    // Create hex dump of string data
    static std::string hexDumpString(const std::string& str);

    // ========================================================================
    // Utility Methods
    // ========================================================================

    // Escape string for JSON (handles quotes, newlines, etc.)
    static std::string escapeJSON(const std::string& str);

    // Convert token type to string
    static std::string tokenTypeToString(TokenType type);

    // Convert AST node type to string
    static std::string nodeTypeToString(ASTNodeType type);

private:
    // Internal helpers for AST serialization
    static std::string serializeBinaryExpr(const BinaryExpr* expr);
    static std::string serializeUnaryExpr(const UnaryExpr* expr);
    static std::string serializeLiteralExpr(const LiteralExpr* expr);
    static std::string serializeIdentifierExpr(const IdentifierExpr* expr);
    static std::string serializeCallExpr(const CallExpr* expr);
    static std::string serializeArrayAccessExpr(const ArrayAccessExpr* expr);

    static std::string serializeIfStmt(const IfStmt* stmt);
    static std::string serializeWhileStmt(const WhileStmt* stmt);
    static std::string serializeReturnStmt(const ReturnStmt* stmt);
    static std::string serializeCompoundStmt(const CompoundStmt* stmt);

    static std::string serializeVarDecl(const VarDecl* decl);
    static std::string serializeFunctionDecl(const FunctionDecl* decl);

    // Helper to create JSON object
    static std::string jsonObject(const std::vector<std::pair<std::string, std::string>>& fields);

    // Helper to create JSON array
    static std::string jsonArray(const std::vector<std::string>& elements);
};

#endif // JSON_SERIALIZER_H
