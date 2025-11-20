#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "scope_manager.h"
#include <vector>
#include <string>

// Semantic error information
struct SemanticError {
    std::string message;
    SourceLocation location;

    SemanticError(const std::string& msg, const SourceLocation& loc)
        : message(msg), location(loc) {}
};

// SemanticAnalyzer - Visitor that walks the AST and performs semantic analysis
// Responsibilities:
// - Register all declarations in appropriate scopes
// - Detect and report redeclaration errors
// - Manage scope entry/exit for blocks, functions, etc.
class SemanticAnalyzer : public ASTVisitor {
private:
    ScopeManager scope_manager_;
    std::vector<SemanticError> errors_;
    bool in_function_scope_;  // Track if we're currently inside a function

    // Helper to add an error
    void add_error(const std::string& message, const SourceLocation& location);

    // Helper to register a symbol and check for redeclaration
    bool register_symbol(const Symbol& symbol, const SourceLocation& location);

public:
    SemanticAnalyzer();

    // Analyze a program (list of declarations)
    void analyze_program(const std::vector<std::unique_ptr<Declaration>>& declarations);

    // Get all semantic errors found
    const std::vector<SemanticError>& get_errors() const { return errors_; }

    // Check if analysis found any errors
    bool has_errors() const { return !errors_.empty(); }

    // Expression visitors
    void visit(BinaryExpr &node) override;
    void visit(UnaryExpr &node) override;
    void visit(LiteralExpr &node) override;
    void visit(IdentifierExpr &node) override;
    void visit(CallExpr &node) override;
    void visit(AssignmentExpr &node) override;
    void visit(ArrayAccessExpr &node) override;
    void visit(MemberAccessExpr &node) override;

    // Statement visitors
    void visit(IfStmt &node) override;
    void visit(WhileStmt &node) override;
    void visit(ForStmt &node) override;
    void visit(ReturnStmt &node) override;
    void visit(CompoundStmt &node) override;
    void visit(ExpressionStmt &node) override;

    // Declaration visitors - these are the main ones for registration
    void visit(VarDecl &node) override;
    void visit(TypeDecl &node) override;
    void visit(StructDecl &node) override;
    void visit(FunctionDecl &node) override;
    void visit(ParameterDecl &node) override;
};

#endif // SEMANTIC_ANALYZER_H
