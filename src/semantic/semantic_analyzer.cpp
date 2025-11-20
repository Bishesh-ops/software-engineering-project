#include "../../include/semantic_analyzer.h"

// ============================================================================
// Constructor and Helpers
// ============================================================================

SemanticAnalyzer::SemanticAnalyzer()
    : in_function_scope_(false) {
}

void SemanticAnalyzer::add_error(const std::string& message, const SourceLocation& location) {
    errors_.push_back(SemanticError(message, location));
}

bool SemanticAnalyzer::register_symbol(const Symbol& symbol, const SourceLocation& location) {
    // Check if symbol already exists in current scope
    if (scope_manager_.exists_in_current_scope(symbol.name)) {
        add_error("Redeclaration of '" + symbol.name + "'", location);
        return false;
    }

    // Insert into current scope
    scope_manager_.insert(symbol);
    return true;
}

void SemanticAnalyzer::analyze_program(const std::vector<std::unique_ptr<Declaration>>& declarations) {
    // Clear any previous errors
    errors_.clear();

    // Process all top-level declarations
    for (const auto& decl : declarations) {
        if (decl) {
            decl->accept(*this);
        }
    }
}

// ============================================================================
// Declaration Visitors - Register symbols in symbol table
// ============================================================================

void SemanticAnalyzer::visit(VarDecl &node) {
    // Create symbol for this variable
    Symbol symbol(
        node.getName(),
        node.getType(),
        scope_manager_.get_current_scope_level(),
        node.getIsArray(),
        node.getArraySize() ? 0 : 0,  // Size would need to be evaluated
        node.getPointerLevel()
    );

    // Register the variable
    register_symbol(symbol, node.getLocation());

    // Visit initializer if present
    if (node.getInitializer()) {
        node.getInitializer()->accept(*this);
    }

    // Visit array size expression if present
    if (node.getArraySize()) {
        node.getArraySize()->accept(*this);
    }
}

void SemanticAnalyzer::visit(TypeDecl &node) {
    // For typedef, we could register it as a type
    // For now, we'll just note it exists but not add to symbol table
    // Future enhancement: maintain a separate type table
}

void SemanticAnalyzer::visit(StructDecl &node) {
    // Register the struct name as a type
    // Future enhancement: maintain struct field information
    // For now, just visit the fields to check for any issues
    for (const auto& field : node.getFields()) {
        if (field) {
            // Note: struct fields are in their own namespace, not the current scope
            // We visit them but don't register in the main symbol table
            // Future enhancement: maintain struct field tables
        }
    }
}

void SemanticAnalyzer::visit(FunctionDecl &node) {
    // Register function in current scope (should be global)
    Symbol func_symbol(
        Symbol::AsFunction,
        node.getName(),
        node.getReturnType(),
        scope_manager_.get_current_scope_level()
    );

    register_symbol(func_symbol, node.getLocation());

    // Enter function scope for parameters and body
    scope_manager_.enter_scope();
    in_function_scope_ = true;

    // Register parameters in function scope
    for (const auto& param : node.getParameters()) {
        if (param) {
            param->accept(*this);
        }
    }

    // Visit function body if present
    if (node.getBody()) {
        node.getBody()->accept(*this);
    }

    // Exit function scope
    in_function_scope_ = false;
    scope_manager_.exit_scope();
}

void SemanticAnalyzer::visit(ParameterDecl &node) {
    // Create symbol for parameter
    Symbol param_symbol(
        node.getName(),
        node.getType(),
        scope_manager_.get_current_scope_level()
    );

    // Register parameter in function scope
    register_symbol(param_symbol, node.getLocation());
}

// ============================================================================
// Statement Visitors - Manage scopes and visit nested declarations
// ============================================================================

void SemanticAnalyzer::visit(CompoundStmt &node) {
    // Enter new block scope
    scope_manager_.enter_scope();

    // Visit all statements in the block
    for (const auto& stmt : node.getStatements()) {
        if (stmt) {
            stmt->accept(*this);
        }
    }

    // Exit block scope
    scope_manager_.exit_scope();
}

void SemanticAnalyzer::visit(IfStmt &node) {
    // Visit condition
    if (node.getCondition()) {
        node.getCondition()->accept(*this);
    }

    // Visit then branch (each branch creates its own scope if it's a compound stmt)
    if (node.getThenBranch()) {
        node.getThenBranch()->accept(*this);
    }

    // Visit else branch if present
    if (node.getElseBranch()) {
        node.getElseBranch()->accept(*this);
    }
}

void SemanticAnalyzer::visit(WhileStmt &node) {
    // Visit condition
    if (node.getCondition()) {
        node.getCondition()->accept(*this);
    }

    // Visit body
    if (node.getBody()) {
        node.getBody()->accept(*this);
    }
}

void SemanticAnalyzer::visit(ForStmt &node) {
    // For loops have their own scope for the initializer
    scope_manager_.enter_scope();

    // Visit initializer (could be a VarDecl)
    if (node.getInitializer()) {
        node.getInitializer()->accept(*this);
    }

    // Visit condition
    if (node.getCondition()) {
        node.getCondition()->accept(*this);
    }

    // Visit increment
    if (node.getIncrement()) {
        node.getIncrement()->accept(*this);
    }

    // Visit body
    if (node.getBody()) {
        node.getBody()->accept(*this);
    }

    scope_manager_.exit_scope();
}

void SemanticAnalyzer::visit(ReturnStmt &node) {
    // Visit return value expression if present
    if (node.getReturnValue()) {
        node.getReturnValue()->accept(*this);
    }
}

void SemanticAnalyzer::visit(ExpressionStmt &node) {
    // Visit the expression
    if (node.getExpression()) {
        node.getExpression()->accept(*this);
    }
}

// ============================================================================
// Expression Visitors - Traverse expressions but don't register anything
// ============================================================================

void SemanticAnalyzer::visit(BinaryExpr &node) {
    // Visit left and right operands
    if (node.getLeft()) {
        node.getLeft()->accept(*this);
    }
    if (node.getRight()) {
        node.getRight()->accept(*this);
    }
}

void SemanticAnalyzer::visit(UnaryExpr &node) {
    // Visit operand
    if (node.getOperand()) {
        node.getOperand()->accept(*this);
    }
}

void SemanticAnalyzer::visit(LiteralExpr &node) {
    // Literals don't need processing for declaration registration
}

void SemanticAnalyzer::visit(IdentifierExpr &node) {
    // Future enhancement: Check if identifier is declared
    // For now, we're just registering declarations
}

void SemanticAnalyzer::visit(CallExpr &node) {
    // Visit callee
    if (node.getCallee()) {
        node.getCallee()->accept(*this);
    }

    // Visit arguments
    for (const auto& arg : node.getArguments()) {
        if (arg) {
            arg->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(AssignmentExpr &node) {
    // Visit target and value
    if (node.getTarget()) {
        node.getTarget()->accept(*this);
    }
    if (node.getValue()) {
        node.getValue()->accept(*this);
    }
}

void SemanticAnalyzer::visit(ArrayAccessExpr &node) {
    // Visit array and index
    if (node.getArray()) {
        node.getArray()->accept(*this);
    }
    if (node.getIndex()) {
        node.getIndex()->accept(*this);
    }
}

void SemanticAnalyzer::visit(MemberAccessExpr &node) {
    // Visit object
    if (node.getObject()) {
        node.getObject()->accept(*this);
    }
}
