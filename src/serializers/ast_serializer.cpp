#include "json_serializers.h"
#include "third_party/json.hpp"
#include <stack>
#include <sstream>

using json = nlohmann::json;

// ============================================================================
// JsonSerializerVisitor - Visitor class for AST to JSON serialization
// ============================================================================

class JsonSerializerVisitor : public ASTVisitor {
private:
    std::stack<json> nodeStack;  // Stack to build JSON tree recursively

    // Helper: Add location info to a JSON object
    void addLocation(json& j, const ASTNode& node) {
        j["location"] = {
            {"file", node.getLocation().filename},
            {"line", node.getLine()},
            {"column", node.getColumn()}
        };
    }

    // Helper: Convert ASTNodeType enum to string
    std::string nodeTypeToString(ASTNodeType type) {
        switch (type) {
            // Expression types
            case ASTNodeType::BINARY_EXPR: return "BINARY_EXPR";
            case ASTNodeType::UNARY_EXPR: return "UNARY_EXPR";
            case ASTNodeType::LITERAL_EXPR: return "LITERAL_EXPR";
            case ASTNodeType::IDENTIFIER_EXPR: return "IDENTIFIER_EXPR";
            case ASTNodeType::CALL_EXPR: return "CALL_EXPR";
            case ASTNodeType::ASSIGNMENT_EXPR: return "ASSIGNMENT_EXPR";
            case ASTNodeType::ARRAY_ACCESS_EXPR: return "ARRAY_ACCESS_EXPR";
            case ASTNodeType::MEMBER_ACCESS_EXPR: return "MEMBER_ACCESS_EXPR";
            case ASTNodeType::TYPE_CAST_EXPR: return "TYPE_CAST_EXPR";

            // Statement types
            case ASTNodeType::IF_STMT: return "IF_STMT";
            case ASTNodeType::WHILE_STMT: return "WHILE_STMT";
            case ASTNodeType::FOR_STMT: return "FOR_STMT";
            case ASTNodeType::RETURN_STMT: return "RETURN_STMT";
            case ASTNodeType::COMPOUND_STMT: return "COMPOUND_STMT";
            case ASTNodeType::EXPRESSION_STMT: return "EXPRESSION_STMT";

            // Declaration types
            case ASTNodeType::VAR_DECL: return "VAR_DECL";
            case ASTNodeType::TYPE_DECL: return "TYPE_DECL";
            case ASTNodeType::STRUCT_DECL: return "STRUCT_DECL";
            case ASTNodeType::FUNCTION_DECL: return "FUNCTION_DECL";
            case ASTNodeType::PARAMETER_DECL: return "PARAMETER_DECL";

            default: return "UNKNOWN";
        }
    }

    // Helper: Convert LiteralType enum to string
    std::string literalTypeToString(LiteralExpr::LiteralType type) {
        switch (type) {
            case LiteralExpr::LiteralType::INTEGER: return "INTEGER";
            case LiteralExpr::LiteralType::FLOAT: return "FLOAT";
            case LiteralExpr::LiteralType::STRING: return "STRING";
            case LiteralExpr::LiteralType::CHAR: return "CHAR";
            case LiteralExpr::LiteralType::BOOLEAN: return "BOOLEAN";
            default: return "UNKNOWN";
        }
    }

public:
    // Get the final result from the stack
    json getResult() {
        if (nodeStack.empty()) {
            return json::object();
        }
        json result = nodeStack.top();
        nodeStack.pop();
        return result;
    }

    // ========================================================================
    // Expression Visitors
    // ========================================================================

    void visit(BinaryExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["operator"] = node.getOperator();

        // Visit left operand
        if (node.getLeft()) {
            node.getLeft()->accept(*this);
            j["left"] = getResult();
        } else {
            j["left"] = nullptr;
        }

        // Visit right operand
        if (node.getRight()) {
            node.getRight()->accept(*this);
            j["right"] = getResult();
        } else {
            j["right"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(UnaryExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["operator"] = node.getOperator();
        j["is_prefix"] = node.isPrefixOp();

        // Visit operand
        if (node.getOperand()) {
            node.getOperand()->accept(*this);
            j["operand"] = getResult();
        } else {
            j["operand"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(LiteralExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["value"] = node.getValue();
        j["literal_type"] = literalTypeToString(node.getLiteralType());

        nodeStack.push(j);
    }

    void visit(IdentifierExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["name"] = node.getName();

        nodeStack.push(j);
    }

    void visit(CallExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit callee
        if (node.getCallee()) {
            node.getCallee()->accept(*this);
            j["callee"] = getResult();
        } else {
            j["callee"] = nullptr;
        }

        // Visit arguments
        json argsArray = json::array();
        for (const auto& arg : node.getArguments()) {
            if (arg) {
                arg->accept(*this);
                argsArray.push_back(getResult());
            }
        }
        j["arguments"] = argsArray;

        nodeStack.push(j);
    }

    void visit(AssignmentExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit target (LHS)
        if (node.getTarget()) {
            node.getTarget()->accept(*this);
            j["target"] = getResult();
        } else {
            j["target"] = nullptr;
        }

        // Visit value (RHS)
        if (node.getValue()) {
            node.getValue()->accept(*this);
            j["value"] = getResult();
        } else {
            j["value"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(ArrayAccessExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit array
        if (node.getArray()) {
            node.getArray()->accept(*this);
            j["array"] = getResult();
        } else {
            j["array"] = nullptr;
        }

        // Visit index
        if (node.getIndex()) {
            node.getIndex()->accept(*this);
            j["index"] = getResult();
        } else {
            j["index"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(MemberAccessExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["member_name"] = node.getMemberName();
        j["is_arrow"] = node.getIsArrow();

        // Visit object
        if (node.getObject()) {
            node.getObject()->accept(*this);
            j["object"] = getResult();
        } else {
            j["object"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(TypeCastExpr& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["target_type"] = node.getTargetType();
        j["is_implicit"] = node.getIsImplicit();

        // Visit operand
        if (node.getOperand()) {
            node.getOperand()->accept(*this);
            j["operand"] = getResult();
        } else {
            j["operand"] = nullptr;
        }

        nodeStack.push(j);
    }

    // ========================================================================
    // Statement Visitors
    // ========================================================================

    void visit(IfStmt& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit condition
        if (node.getCondition()) {
            node.getCondition()->accept(*this);
            j["condition"] = getResult();
        } else {
            j["condition"] = nullptr;
        }

        // Visit then branch
        if (node.getThenBranch()) {
            node.getThenBranch()->accept(*this);
            j["then_branch"] = getResult();
        } else {
            j["then_branch"] = nullptr;
        }

        // Visit else branch (optional)
        if (node.getElseBranch()) {
            node.getElseBranch()->accept(*this);
            j["else_branch"] = getResult();
        } else {
            j["else_branch"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(WhileStmt& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit condition
        if (node.getCondition()) {
            node.getCondition()->accept(*this);
            j["condition"] = getResult();
        } else {
            j["condition"] = nullptr;
        }

        // Visit body
        if (node.getBody()) {
            node.getBody()->accept(*this);
            j["body"] = getResult();
        } else {
            j["body"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(ForStmt& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit initializer (optional)
        if (node.getInitializer()) {
            node.getInitializer()->accept(*this);
            j["initializer"] = getResult();
        } else {
            j["initializer"] = nullptr;
        }

        // Visit condition (optional)
        if (node.getCondition()) {
            node.getCondition()->accept(*this);
            j["condition"] = getResult();
        } else {
            j["condition"] = nullptr;
        }

        // Visit increment (optional)
        if (node.getIncrement()) {
            node.getIncrement()->accept(*this);
            j["increment"] = getResult();
        } else {
            j["increment"] = nullptr;
        }

        // Visit body
        if (node.getBody()) {
            node.getBody()->accept(*this);
            j["body"] = getResult();
        } else {
            j["body"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(ReturnStmt& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit return value (optional for void returns)
        if (node.getReturnValue()) {
            node.getReturnValue()->accept(*this);
            j["return_value"] = getResult();
        } else {
            j["return_value"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(CompoundStmt& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit all statements
        json stmtsArray = json::array();
        for (const auto& stmt : node.getStatements()) {
            if (stmt) {
                stmt->accept(*this);
                stmtsArray.push_back(getResult());
            }
        }
        j["statements"] = stmtsArray;

        nodeStack.push(j);
    }

    void visit(ExpressionStmt& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);

        // Visit expression
        if (node.getExpression()) {
            node.getExpression()->accept(*this);
            j["expression"] = getResult();
        } else {
            j["expression"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(DeclStmt& node) override {
        json j;
        j["node_type"] = "DECL_STMT";  // Not in ASTNodeType enum
        addLocation(j, node);

        // Visit declaration
        if (node.getDeclaration()) {
            node.getDeclaration()->accept(*this);
            j["declaration"] = getResult();
        } else {
            j["declaration"] = nullptr;
        }

        nodeStack.push(j);
    }

    // ========================================================================
    // Declaration Visitors
    // ========================================================================

    void visit(VarDecl& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["name"] = node.getName();
        j["type"] = node.getType();
        j["is_array"] = node.getIsArray();
        j["pointer_level"] = node.getPointerLevel();

        // Visit initializer (optional)
        if (node.getInitializer()) {
            node.getInitializer()->accept(*this);
            j["initializer"] = getResult();
        } else {
            j["initializer"] = nullptr;
        }

        // Visit array size (optional)
        if (node.getArraySize()) {
            node.getArraySize()->accept(*this);
            j["array_size"] = getResult();
        } else {
            j["array_size"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(TypeDecl& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["name"] = node.getName();
        j["underlying_type"] = node.getUnderlyingType();

        nodeStack.push(j);
    }

    void visit(StructDecl& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["name"] = node.getName();

        // Visit all fields
        json fieldsArray = json::array();
        for (const auto& field : node.getFields()) {
            if (field) {
                field->accept(*this);
                fieldsArray.push_back(getResult());
            }
        }
        j["fields"] = fieldsArray;

        nodeStack.push(j);
    }

    void visit(FunctionDecl& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["name"] = node.getName();
        j["return_type"] = node.getReturnType();

        // Visit parameters
        json paramsArray = json::array();
        for (const auto& param : node.getParameters()) {
            if (param) {
                param->accept(*this);
                paramsArray.push_back(getResult());
            }
        }
        j["parameters"] = paramsArray;

        // Visit body (optional for forward declarations)
        if (node.getBody()) {
            node.getBody()->accept(*this);
            j["body"] = getResult();
        } else {
            j["body"] = nullptr;
        }

        nodeStack.push(j);
    }

    void visit(ParameterDecl& node) override {
        json j;
        j["node_type"] = nodeTypeToString(node.getNodeType());
        addLocation(j, node);
        j["name"] = node.getName();
        j["type"] = node.getType();

        nodeStack.push(j);
    }
};

// ============================================================================
// Public API Implementation
// ============================================================================

std::string serializeAstToJson(const std::vector<std::unique_ptr<Declaration>>& ast) {
    json result;

    // Add metadata
    result["stage"] = "parsing";
    result["declaration_count"] = ast.size();

    // Serialize all top-level declarations
    json declArray = json::array();
    JsonSerializerVisitor visitor;

    for (const auto& decl : ast) {
        if (decl) {
            decl->accept(visitor);
            declArray.push_back(visitor.getResult());
        }
    }

    result["declarations"] = declArray;

    // Convert to string with pretty printing (2-space indentation)
    return result.dump(2);
}
