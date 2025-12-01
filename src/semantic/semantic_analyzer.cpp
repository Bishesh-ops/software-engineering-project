#include "../../include/semantic_analyzer.h"

// ============================================================================
// Constructor and Helpers
// ============================================================================

SemanticAnalyzer::SemanticAnalyzer()
    : in_function_scope_(false), warnings_enabled_(true) {
}

void SemanticAnalyzer::add_error(const std::string& message, const SourceLocation& location) {
    error_handler_.error(message, location);
}

void SemanticAnalyzer::add_warning(const std::string& message, const SourceLocation& location) {
    // Only emit warning if warnings are enabled
    if (warnings_enabled_) {
        error_handler_.warning(message, location);
    }
}

bool SemanticAnalyzer::register_symbol(const Symbol& symbol, const SourceLocation& location) {
    // Check if symbol already exists in current scope
    if (scope_manager_.exists_in_current_scope(symbol.name)) {
        add_error("Redeclaration of '" + symbol.name + "'", location);
        return false;
    }

    // Store declaration location in the symbol
    Symbol sym_with_location = symbol;
    sym_with_location.declaration_location = location;

    // Insert into current scope
    scope_manager_.insert(sym_with_location);
    return true;
}

void SemanticAnalyzer::analyze_program(const std::vector<std::unique_ptr<Declaration>>& declarations) {
    // Clear any previous errors, warnings, and expression types
    error_handler_.clear();
    expression_types_.clear();

    // Process all top-level declarations
    for (const auto& decl : declarations) {
        // Stop if we've reached the maximum error limit (error recovery)
        if (error_handler_.has_reached_max_errors()) {
            break;
        }

        if (decl) {
            decl->accept(*this);
        }
    }
}

std::shared_ptr<Type> SemanticAnalyzer::get_expression_type(const Expression* expr) const {
    auto it = expression_types_.find(expr);
    if (it != expression_types_.end()) {
        return it->second;
    }
    return nullptr;
}

void SemanticAnalyzer::set_expression_type(const Expression* expr, std::shared_ptr<Type> type) {
    expression_types_[expr] = type;
}

bool SemanticAnalyzer::is_lvalue(const Expression* expr) const {
    if (!expr) return false;

    // Check the AST node type
    ASTNodeType node_type = expr->getNodeType();

    // Lvalues are expressions that refer to memory locations:
    // - Identifiers (variables)
    // - Array access (arr[i])
    // - Member access (obj.member, ptr->member)
    // - Dereferenced pointers (*ptr)

    if (node_type == ASTNodeType::IDENTIFIER_EXPR) {
        return true;
    }

    if (node_type == ASTNodeType::ARRAY_ACCESS_EXPR) {
        return true;
    }

    if (node_type == ASTNodeType::MEMBER_ACCESS_EXPR) {
        return true;
    }

    if (node_type == ASTNodeType::UNARY_EXPR) {
        // Dereference operator (*ptr) is an lvalue
        const UnaryExpr* unary = static_cast<const UnaryExpr*>(expr);
        if (unary->getOperator() == "*") {
            return true;
        }
    }

    // Everything else (literals, function calls, binary expressions, etc.) is not an lvalue
    return false;
}

// Calculate Levenshtein distance between two strings (for finding similar identifiers)
int SemanticAnalyzer::levenshtein_distance(const std::string& s1, const std::string& s2) {
    const size_t m = s1.size();
    const size_t n = s2.size();

    if (m == 0) return n;
    if (n == 0) return m;

    // Create distance matrix
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    // Initialize base cases
    for (size_t i = 0; i <= m; ++i) dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j) dp[0][j] = j;

    // Fill the matrix
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j],      // deletion
                                          dp[i][j - 1],      // insertion
                                          dp[i - 1][j - 1]}); // substitution
            }
        }
    }

    return dp[m][n];
}

// Find similar identifier name for suggestions
std::string SemanticAnalyzer::find_similar_identifier(const std::string& name) const {
    auto all_names = scope_manager_.get_all_symbol_names();

    if (all_names.empty()) {
        return "";
    }

    std::string best_match;
    int best_distance = INT_MAX;

    for (const auto& candidate : all_names) {
        int distance = levenshtein_distance(name, candidate);

        // Consider it a good suggestion if:
        // 1. Distance is small (≤ 2 for short names, ≤ 3 for longer)
        // 2. It's the closest match so far
        int max_distance = (name.length() <= 4) ? 2 : 3;

        if (distance <= max_distance && distance < best_distance) {
            best_distance = distance;
            best_match = candidate;
        }
    }

    return best_match;
}

// ============================================================================
// Declaration Visitors - Register symbols in symbol table
// ============================================================================

void SemanticAnalyzer::visit(VarDecl &node) {
    // USER STORY #13: Check if the type is a struct type
    std::shared_ptr<Type> var_type;

    // Check if this is a struct type
    // Type string might be "struct Point" or just "Point"
    std::string type_name = node.getType();

    // Strip "struct " prefix if present
    const std::string struct_prefix = "struct ";
    if (type_name.find(struct_prefix) == 0) {
        type_name = type_name.substr(struct_prefix.length());
    }

    auto struct_it = struct_types_.find(type_name);
    if (struct_it != struct_types_.end()) {
        // It's a struct type - use the registered struct type
        var_type = struct_it->second;

        // Handle pointers to structs
        if (node.getPointerLevel() > 0) {
            var_type = Type::makePointer(Type::BaseType::STRUCT, node.getPointerLevel());
            // Note: This loses struct member information for pointers
            // A better approach would be to store the struct name with the pointer
        }
    } else {
        // Regular type - convert from string
        var_type = Type::fromString(node.getType());

        // Handle pointer level
        if (node.getPointerLevel() > 0 && var_type) {
            var_type = Type::makePointer(var_type->getBaseType(), node.getPointerLevel());
        }
        // Handle arrays
        else if (node.getIsArray() && var_type) {
            var_type = Type::makeArray(var_type->getBaseType(), 0);  // Size TBD
        }
    }

    // Create symbol for this variable
    Symbol symbol(
        node.getName(),
        var_type,
        scope_manager_.get_current_scope_level()
    );

    // Register the variable
    register_symbol(symbol, node.getLocation());

    // Visit initializer if present and check type compatibility
    if (node.getInitializer()) {
        node.getInitializer()->accept(*this);

        // Get the declared type and initializer type
        auto declared_type = symbol.symbol_type;
        auto init_type = get_expression_type(node.getInitializer());

        if (declared_type && init_type) {
            // Check if types are compatible
            if (!declared_type->equals(*init_type)) {
                // Check if implicit conversion is allowed
                if (init_type->canConvertTo(*declared_type)) {
                    // Allow conversion but warn for potentially lossy conversions
                    // (e.g., float to int)
                    if (init_type->isFloatingPoint() && declared_type->isIntegral()) {
                        add_warning("Implicit conversion from '" + init_type->toString() +
                                    "' to '" + declared_type->toString() +
                                    "' may lose precision",
                                    node.getLocation());
                    }
                } else {
                    // Types are incompatible
                    add_error("Cannot initialize variable '" + node.getName() +
                              "' of type '" + declared_type->toString() +
                              "' with value of type '" + init_type->toString() + "'",
                              node.getLocation());
                }
            }
        }
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
    (void)node; // Suppress unused parameter warning
}

void SemanticAnalyzer::visit(StructDecl &node) {
    // USER STORY #13: Register the struct type with its members

    // Build the list of struct members
    std::vector<Type::StructMember> members;
    for (const auto& field : node.getFields()) {
        if (field) {
            // Convert field to struct member
            auto field_type = Type::fromString(field->getType());

            // Handle pointer fields
            if (field->isPointer()) {
                field_type = Type::makePointer(field_type->getBaseType(), field->getPointerLevel());
            }
            // Handle array fields
            else if (field->getIsArray()) {
                // Note: array size would need to be evaluated from the expression
                field_type = Type::makeArray(field_type->getBaseType(), 0);  // 0 for now
            }

            members.emplace_back(field->getName(), field_type);
        }
    }

    // Create and register the struct type
    auto struct_type = Type::makeStruct(node.getName(), members);
    struct_types_[node.getName()] = struct_type;

    // Also register the struct name as a symbol (for type checking)
    // Note: In C, struct tags are in a separate namespace, but we'll keep it simple
}

void SemanticAnalyzer::visit(FunctionDecl &node) {
    // Collect parameter types
    std::vector<std::shared_ptr<Type>> param_types;
    for (const auto& param : node.getParameters()) {
        if (param) {
            auto param_type = Type::fromString(param->getType());
            if (param_type) {
                param_types.push_back(param_type);
            }
        }
    }

    // Register function in current scope (should be global) with parameter types
    Symbol func_symbol(
        Symbol::AsFunction,
        node.getName(),
        Type::fromString(node.getReturnType()),
        param_types,
        scope_manager_.get_current_scope_level()
    );

    register_symbol(func_symbol, node.getLocation());

    // Set current function context for return type checking
    current_function_name_ = node.getName();
    current_function_return_type_ = Type::fromString(node.getReturnType());
    current_function_has_return_ = false;

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

    // Check if non-void function has a return statement
    if (current_function_return_type_ &&
        !current_function_return_type_->isVoid() &&
        !current_function_has_return_) {
        add_warning("Non-void function '" + node.getName() +
                    "' does not return a value",
                    node.getLocation());
    }

    // Check for unused variables (parameters and locals) before exiting function scope
    auto unused_vars = scope_manager_.get_unused_variables_in_current_scope();
    for (const auto& var : unused_vars) {
        add_warning("Unused variable '" + var.name + "'", var.declaration_location);
    }

    // Exit function scope and clear function context
    in_function_scope_ = false;
    current_function_name_ = "";
    current_function_return_type_ = nullptr;
    current_function_has_return_ = false;
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

    // Check for unused variables before exiting scope
    auto unused_vars = scope_manager_.get_unused_variables_in_current_scope();
    for (const auto& var : unused_vars) {
        add_warning("Unused variable '" + var.name + "'", var.declaration_location);
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
    // Mark that we've seen a return statement
    current_function_has_return_ = true;

    // Visit return value expression if present
    if (node.getReturnValue()) {
        node.getReturnValue()->accept(*this);
    }

    // Validate return type matches function signature
    if (!current_function_return_type_) {
        // Not in a function context - this shouldn't happen but handle gracefully
        return;
    }

    // Case 1: void function with return value
    if (current_function_return_type_->isVoid() && node.getReturnValue()) {
        add_error("Void function '" + current_function_name_ +
                  "' should not return a value",
                  node.getLocation());
        return;
    }

    // Case 2: non-void function without return value
    if (!current_function_return_type_->isVoid() && !node.getReturnValue()) {
        add_error("Non-void function '" + current_function_name_ +
                  "' must return a value",
                  node.getLocation());
        return;
    }

    // Case 3: non-void function with return value - check type compatibility
    if (!current_function_return_type_->isVoid() && node.getReturnValue()) {
        auto return_type = get_expression_type(node.getReturnValue());

        if (return_type) {
            // Check if types match
            if (!current_function_return_type_->equals(*return_type)) {
                // Check if implicit conversion is allowed
                if (return_type->canConvertTo(*current_function_return_type_)) {
                    // Allow conversion but warn for lossy conversions
                    if (return_type->isFloatingPoint() && current_function_return_type_->isIntegral()) {
                        add_warning("Returning '" + return_type->toString() +
                                    "' from function with return type '" +
                                    current_function_return_type_->toString() +
                                    "' may lose precision",
                                    node.getLocation());
                    }
                } else {
                    // Types are incompatible
                    add_error("Return type mismatch: expected '" +
                              current_function_return_type_->toString() +
                              "', got '" + return_type->toString() + "'",
                              node.getLocation());
                }
            }
        }
    }
}

void SemanticAnalyzer::visit(ExpressionStmt &node) {
    // Visit the expression
    if (node.getExpression()) {
        node.getExpression()->accept(*this);
    }
}

void SemanticAnalyzer::visit(DeclStmt &node) {
    // Visit the wrapped declaration - this will register the variable in the symbol table
    if (node.getDeclaration()) {
        node.getDeclaration()->accept(*this);
    }
}

// ============================================================================
// Expression Visitors - Traverse expressions but don't register anything
// ============================================================================

void SemanticAnalyzer::visit(BinaryExpr &node) {
    // Visit left and right operands first to get their types
    if (node.getLeft()) {
        node.getLeft()->accept(*this);
    }
    if (node.getRight()) {
        node.getRight()->accept(*this);
    }

    // Get types of operands
    auto left_type = get_expression_type(node.getLeft());
    auto right_type = get_expression_type(node.getRight());

    // If either operand doesn't have a type, we can't check the operation
    if (!left_type || !right_type) {
        return;
    }

    const std::string& op = node.getOperator();

    // USER STORY #11: Apply array-to-pointer decay if needed
    auto left_type_decayed = applyArrayToPointerDecay(left_type);
    auto right_type_decayed = applyArrayToPointerDecay(right_type);

    // If array decay occurred, insert implicit conversion nodes
    if (!left_type->equals(*left_type_decayed)) {
        auto cast_node = std::make_unique<TypeCastExpr>(
            node.releaseLeft(),
            left_type_decayed->toString(),
            true,  // implicit conversion
            node.getLocation()
        );
        set_expression_type(cast_node.get(), left_type_decayed);
        node.setLeft(std::move(cast_node));
        left_type = left_type_decayed;
    }

    if (!right_type->equals(*right_type_decayed)) {
        auto cast_node = std::make_unique<TypeCastExpr>(
            node.releaseRight(),
            right_type_decayed->toString(),
            true,  // implicit conversion
            node.getLocation()
        );
        set_expression_type(cast_node.get(), right_type_decayed);
        node.setRight(std::move(cast_node));
        right_type = right_type_decayed;
    }

    // Check if the binary operator is valid for these operand types
    if (!isValidBinaryOperator(*left_type, *right_type, op)) {
        // USER STORY #12: Provide specific error messages for pointer arithmetic
        std::string error_msg;

        // Special error messages for pointer operations
        if (left_type->isPointer() || right_type->isPointer()) {
            if (op == "*") {
                error_msg = "Invalid operation: cannot multiply pointers ('" +
                            left_type->toString() + "' * '" + right_type->toString() + "')";
            }
            else if (op == "/") {
                error_msg = "Invalid operation: cannot divide pointers ('" +
                            left_type->toString() + "' / '" + right_type->toString() + "')";
            }
            else if (op == "%") {
                error_msg = "Invalid operation: cannot use modulo with pointers ('" +
                            left_type->toString() + "' % '" + right_type->toString() + "')";
            }
            else if (op == "-" && left_type->isPointer() && right_type->isPointer()) {
                error_msg = "Invalid operation: cannot subtract pointers of different types ('" +
                            left_type->toString() + "' - '" + right_type->toString() + "')";
            }
            else if (op == "+" && left_type->isPointer() && right_type->isPointer()) {
                error_msg = "Invalid operation: cannot add two pointers ('" +
                            left_type->toString() + "' + '" + right_type->toString() + "')";
            }
            else if ((op == "+" || op == "-") && left_type->isPointer() && !right_type->isIntegral()) {
                error_msg = "Invalid operation: pointer arithmetic requires integer operand ('" +
                            left_type->toString() + "' " + op + " '" + right_type->toString() + "')";
            }
            else if ((op == "+" || op == "-") && right_type->isPointer() && !left_type->isIntegral()) {
                error_msg = "Invalid operation: pointer arithmetic requires integer operand ('" +
                            left_type->toString() + "' " + op + " '" + right_type->toString() + "')";
            }
            else if ((op == "+" || op == "-") && left_type->isPointer() &&
                     left_type->getBaseType() == Type::BaseType::VOID) {
                error_msg = "Invalid operation: arithmetic on void pointer ('" +
                            left_type->toString() + "' " + op + " '" + right_type->toString() + "')";
            }
            else if ((op == "+" || op == "-") && right_type->isPointer() &&
                     right_type->getBaseType() == Type::BaseType::VOID) {
                error_msg = "Invalid operation: arithmetic on void pointer ('" +
                            left_type->toString() + "' " + op + " '" + right_type->toString() + "')";
            }
            else {
                error_msg = "Type error: invalid operands to binary " + op +
                            " ('" + left_type->toString() + "' and '" +
                            right_type->toString() + "')";
            }
        }
        else {
            error_msg = "Type error: invalid operands to binary " + op +
                        " ('" + left_type->toString() + "' and '" +
                        right_type->toString() + "')";
        }

        add_error(error_msg, node.getLocation());
        return;
    }

    // Determine the result type
    std::shared_ptr<Type> result_type;

    // For arithmetic operators, use type promotion rules and insert conversion nodes
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        // USER STORY #11: Apply implicit conversions for arithmetic operations
        // Get the common type (applies integer promotion + usual arithmetic conversions)
        auto common_type = get_common_type(left_type, right_type);

        if (common_type) {
            // Insert conversion nodes if types don't match the common type
            if (!left_type->equals(*common_type)) {
                auto cast_node = std::make_unique<TypeCastExpr>(
                    node.releaseLeft(),
                    common_type->toString(),
                    true,  // implicit conversion
                    node.getLocation()
                );
                set_expression_type(cast_node.get(), common_type);
                node.setLeft(std::move(cast_node));
            }

            if (!right_type->equals(*common_type)) {
                auto cast_node = std::make_unique<TypeCastExpr>(
                    node.releaseRight(),
                    common_type->toString(),
                    true,  // implicit conversion
                    node.getLocation()
                );
                set_expression_type(cast_node.get(), common_type);
                node.setRight(std::move(cast_node));
            }
        }

        result_type = getArithmeticResultType(*left_type, *right_type, op);
    }
    // For comparison operators, result is int (representing boolean in C)
    else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
        // USER STORY #11: Apply usual arithmetic conversions for comparisons
        if (left_type->isArithmetic() && right_type->isArithmetic()) {
            auto common_type = get_common_type(left_type, right_type);

            if (common_type) {
                if (!left_type->equals(*common_type)) {
                    auto cast_node = std::make_unique<TypeCastExpr>(
                        node.releaseLeft(),
                        common_type->toString(),
                        true,
                        node.getLocation()
                    );
                    set_expression_type(cast_node.get(), common_type);
                    node.setLeft(std::move(cast_node));
                }

                if (!right_type->equals(*common_type)) {
                    auto cast_node = std::make_unique<TypeCastExpr>(
                        node.releaseRight(),
                        common_type->toString(),
                        true,
                        node.getLocation()
                    );
                    set_expression_type(cast_node.get(), common_type);
                    node.setRight(std::move(cast_node));
                }
            }
        }

        result_type = Type::makeInt();
    }
    // For logical operators, result is int (representing boolean in C)
    else if (op == "&&" || op == "||") {
        result_type = Type::makeInt();
    }
    // For bitwise operators, result follows integer promotion
    else if (op == "&" || op == "|" || op == "^" || op == "<<" || op == ">>") {
        // USER STORY #11: Apply integer promotion for bitwise operators
        auto left_promoted = apply_integer_promotion(left_type);
        auto right_promoted = apply_integer_promotion(right_type);

        if (!left_type->equals(*left_promoted)) {
            auto cast_node = std::make_unique<TypeCastExpr>(
                node.releaseLeft(),
                left_promoted->toString(),
                true,
                node.getLocation()
            );
            set_expression_type(cast_node.get(), left_promoted);
            node.setLeft(std::move(cast_node));
        }

        if (!right_type->equals(*right_promoted)) {
            auto cast_node = std::make_unique<TypeCastExpr>(
                node.releaseRight(),
                right_promoted->toString(),
                true,
                node.getLocation()
            );
            set_expression_type(cast_node.get(), right_promoted);
            node.setRight(std::move(cast_node));
        }

        result_type = getArithmeticResultType(*left_type, *right_type, op);
    }

    if (result_type) {
        set_expression_type(&node, result_type);
    }
}

void SemanticAnalyzer::visit(UnaryExpr &node) {
    // Visit operand
    if (node.getOperand()) {
        node.getOperand()->accept(*this);
    }

    std::string op = node.getOperator();
    auto operand_type = get_expression_type(node.getOperand());

    if (!operand_type) {
        return;  // Can't check if operand type is unknown
    }

    // Validate address-of operator (&)
    if (op == "&") {
        // Address-of can only be applied to lvalues
        if (!is_lvalue(node.getOperand())) {
            add_error("Cannot take address of rvalue (non-lvalue expression)", node.getLocation());
            return;
        }

        // Result type is pointer to operand type
        auto result_type = Type::makePointer(operand_type->getBaseType(), operand_type->getPointerDepth() + 1);
        set_expression_type(&node, result_type);
        return;
    }

    // Validate dereference operator (*)
    if (op == "*") {
        // Dereference requires a pointer type
        if (!operand_type->isPointer()) {
            add_error("Cannot dereference non-pointer type '" + operand_type->toString() + "'", node.getLocation());
            return;
        }

        // Result type is the pointed-to type
        if (operand_type->getPointerDepth() > 0) {
            auto result_type = Type::makePointer(operand_type->getBaseType(), operand_type->getPointerDepth() - 1);
            set_expression_type(&node, result_type);
        }
        return;
    }

    // Validate increment/decrement operators (++, --)
    if (op == "++" || op == "--") {
        // Increment/decrement require lvalues
        if (!is_lvalue(node.getOperand())) {
            add_error("Increment/decrement operator requires an lvalue", node.getLocation());
            return;
        }

        // Increment/decrement require arithmetic or pointer types
        if (!operand_type->isArithmetic() && !operand_type->isPointer()) {
            add_error("Increment/decrement operator requires arithmetic or pointer type", node.getLocation());
            return;
        }

        // Result type is same as operand type
        set_expression_type(&node, operand_type);
        return;
    }

    // Validate unary minus and plus (-, +)
    if (op == "-" || op == "+") {
        // Unary minus/plus require arithmetic types
        if (!operand_type->isArithmetic()) {
            add_error("Unary " + op + " requires arithmetic type", node.getLocation());
            return;
        }

        // Result type is same as operand type
        set_expression_type(&node, operand_type);
        return;
    }

    // Validate logical NOT (!)
    if (op == "!") {
        // Logical NOT can be applied to any scalar type (arithmetic or pointer)
        // Result type is int (boolean in C is int)
        set_expression_type(&node, Type::makeInt());
        return;
    }

    // Validate bitwise NOT (~)
    if (op == "~") {
        // Bitwise NOT requires integral type
        if (!operand_type->isIntegral()) {
            add_error("Bitwise NOT requires integral type", node.getLocation());
            return;
        }

        // Result type is same as operand type
        set_expression_type(&node, operand_type);
        return;
    }
}

void SemanticAnalyzer::visit(LiteralExpr &node) {
    // Infer type from literal
    std::shared_ptr<Type> lit_type;

    switch (node.getLiteralType()) {
        case LiteralExpr::LiteralType::INTEGER:
            lit_type = Type::makeInt();
            break;
        case LiteralExpr::LiteralType::FLOAT:
            lit_type = Type::makeFloat();
            break;
        case LiteralExpr::LiteralType::CHAR:
            lit_type = Type::makeChar();
            break;
        case LiteralExpr::LiteralType::STRING:
            // String literals are char* in C
            lit_type = Type::makePointer(Type::BaseType::CHAR, 1);
            break;
        case LiteralExpr::LiteralType::BOOLEAN:
            lit_type = Type::makeInt(); // Booleans are int in C
            break;
    }

    if (lit_type) {
        set_expression_type(&node, lit_type);
    }
}

void SemanticAnalyzer::visit(IdentifierExpr &node) {
    // Look up identifier in symbol table
    auto symbol_opt = scope_manager_.lookup(node.getName());

    if (symbol_opt.has_value()) {
        // Mark the symbol as used (for unused variable warnings)
        scope_manager_.mark_symbol_as_used(node.getName());

        // Set the expression type from the symbol's type
        if (symbol_opt->symbol_type) {
            set_expression_type(&node, symbol_opt->symbol_type);
        }
    } else {
        // Identifier not found - report undeclared identifier error
        std::string error_msg = "Undeclared identifier '" + node.getName() + "'";

        // Try to find a similar identifier for suggestions
        std::string suggestion = find_similar_identifier(node.getName());
        if (!suggestion.empty()) {
            error_msg += "; did you mean '" + suggestion + "'?";
        }

        add_error(error_msg, node.getLocation());
    }
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

    // Check if callee is an identifier (function name)
    const IdentifierExpr* callee_id = dynamic_cast<const IdentifierExpr*>(node.getCallee());
    if (!callee_id) {
        // For now, we only handle direct function calls (not function pointers)
        return;
    }

    // Look up the function in symbol table
    auto func_symbol_opt = scope_manager_.lookup(callee_id->getName());
    if (!func_symbol_opt.has_value()) {
        // Function not declared - this will be caught by undeclared identifier checking
        return;
    }

    const Symbol& func_symbol = func_symbol_opt.value();

    // Check if it's actually a function
    if (!func_symbol.is_function) {
        add_error("'" + callee_id->getName() + "' is not a function", node.getLocation());
        return;
    }

    // Get the expected parameter types
    const auto& expected_params = func_symbol.parameter_types;
    const auto& actual_args = node.getArguments();

    // Check argument count
    if (actual_args.size() < expected_params.size()) {
        add_error("Too few arguments to function '" + callee_id->getName() +
                  "': expected " + std::to_string(expected_params.size()) +
                  ", got " + std::to_string(actual_args.size()),
                  node.getLocation());
        return;
    }

    if (actual_args.size() > expected_params.size()) {
        add_error("Too many arguments to function '" + callee_id->getName() +
                  "': expected " + std::to_string(expected_params.size()) +
                  ", got " + std::to_string(actual_args.size()),
                  node.getLocation());
        return;
    }

    // Check argument types
    for (size_t i = 0; i < expected_params.size(); ++i) {
        auto expected_type = expected_params[i];
        auto actual_type = get_expression_type(actual_args[i].get());

        if (!expected_type || !actual_type) {
            continue;  // Can't check if type info is missing
        }

        // Check if types match exactly
        if (!expected_type->equals(*actual_type)) {
            // Check if implicit conversion is allowed
            if (!actual_type->canConvertTo(*expected_type)) {
                add_error("Type mismatch for argument " + std::to_string(i + 1) +
                          " of function '" + callee_id->getName() +
                          "': expected '" + expected_type->toString() +
                          "', got '" + actual_type->toString() + "'",
                          node.getLocation());
            } else {
                // Allow conversion but warn for lossy conversions
                if (actual_type->isFloatingPoint() && expected_type->isIntegral()) {
                    add_warning("Implicit conversion from '" + actual_type->toString() +
                                "' to '" + expected_type->toString() +
                                "' for argument " + std::to_string(i + 1) +
                                " may lose precision",
                                node.getLocation());
                }
            }
        }
    }

    // Set the result type of the call expression to the return type
    if (func_symbol.symbol_type) {
        set_expression_type(&node, func_symbol.symbol_type);
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

    // Check if target is an lvalue
    if (node.getTarget() && !is_lvalue(node.getTarget())) {
        add_error("Assignment target must be an lvalue", node.getLocation());
        return;
    }

    // Get types of target and value
    auto target_type = get_expression_type(node.getTarget());
    auto value_type = get_expression_type(node.getValue());

    // If either doesn't have a type, we can't check compatibility
    if (!target_type || !value_type) {
        return;
    }

    // Check type compatibility
    if (!target_type->equals(*value_type)) {
        // Check if implicit conversion is allowed
        if (value_type->canConvertTo(*target_type)) {
            // Allow conversion but warn for potentially lossy conversions
            // (e.g., float to int)
            if (value_type->isFloatingPoint() && target_type->isIntegral()) {
                add_warning("Implicit conversion from '" + value_type->toString() +
                            "' to '" + target_type->toString() +
                            "' may lose precision",
                            node.getLocation());
            }
        } else {
            // Types are incompatible
            add_error("Cannot assign value of type '" + value_type->toString() +
                      "' to variable of type '" + target_type->toString() + "'",
                      node.getLocation());
            return;
        }
    }

    // The result type of an assignment is the type of the target
    set_expression_type(&node, target_type);
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
    // USER STORY #13: Validate struct member access

    // Visit object
    if (node.getObject()) {
        node.getObject()->accept(*this);
    }

    // Get the type of the object being accessed
    auto object_type = get_expression_type(node.getObject());
    if (!object_type) {
        return;  // Type unknown, can't validate
    }

    const std::string& member_name = node.getMemberName();
    bool is_arrow = node.getIsArrow();

    // Validate arrow operator (->)
    if (is_arrow) {
        // Arrow operator requires a pointer to struct
        if (!object_type->isPointer()) {
            add_error("Member access with '->' requires pointer type, got '" +
                      object_type->toString() + "'",
                      node.getLocation());
            return;
        }

        // Get the pointed-to type
        if (object_type->getPointerDepth() > 0) {
            // Dereference the pointer to get struct type
            auto pointed_type = Type::makePointer(
                object_type->getBaseType(),
                object_type->getPointerDepth() - 1
            );

            // If it's a single pointer, we get the base type
            if (object_type->getPointerDepth() == 1) {
                // Check if it's a pointer to struct
                if (object_type->getBaseType() != Type::BaseType::STRUCT) {
                    // Need to look up struct type by name
                    // For now, check if base type is struct
                    pointed_type = std::make_shared<Type>(object_type->getBaseType());
                }
            }

            object_type = pointed_type;
        }
    }
    else {
        // Dot operator (.) requires struct type (not pointer)
        if (object_type->isPointer()) {
            add_error("Member access with '.' on pointer type '" +
                      object_type->toString() + "'; did you mean '->'?",
                      node.getLocation());
            return;
        }
    }

    // Check if the object is a struct type
    if (!object_type->isStruct()) {
        add_error("Member access on non-struct type '" + object_type->toString() + "'",
                  node.getLocation());
        return;
    }

    // Check if the member exists in the struct
    if (!object_type->hasMember(member_name)) {
        add_error("Struct '" + object_type->getStructName() +
                  "' has no member named '" + member_name + "'",
                  node.getLocation());
        return;
    }

    // Get and set the type of the member
    auto member_type = object_type->getMemberType(member_name);
    if (member_type) {
        set_expression_type(&node, member_type);
    }
}

void SemanticAnalyzer::visit(TypeCastExpr &node) {
    // Visit operand
    if (node.getOperand()) {
        node.getOperand()->accept(*this);
    }

    // Get operand type
    auto operand_type = get_expression_type(node.getOperand());
    if (!operand_type) {
        return;
    }

    // Determine the target type
    auto target_type = Type::fromString(node.getTargetType());
    if (!target_type) {
        add_error("Unknown type '" + node.getTargetType() + "' in type cast",
                  node.getLocation());
        return;
    }

    // Set the result type to the target type
    set_expression_type(&node, target_type);

    // For implicit casts, we can add warnings if needed
    if (node.getIsImplicit()) {
        // Check for potentially lossy conversions
        if (operand_type->isFloatingPoint() && target_type->isIntegral()) {
            add_warning("Implicit conversion from '" + operand_type->toString() +
                        "' to '" + target_type->toString() +
                        "' may lose precision",
                        node.getLocation());
        }
    }
}

// ============================================================================
// USER STORY #11: Implicit Type Conversion Helpers
// ============================================================================

std::shared_ptr<Type> SemanticAnalyzer::apply_integer_promotion(std::shared_ptr<Type> type) const {
    return applyIntegerPromotion(type);
}

std::shared_ptr<Type> SemanticAnalyzer::get_common_type(std::shared_ptr<Type> left,
                                                        std::shared_ptr<Type> right) const {
    return getCommonArithmeticType(left, right);
}

std::shared_ptr<Type> SemanticAnalyzer::needs_conversion(std::shared_ptr<Type> from,
                                                         std::shared_ptr<Type> to) const {
    if (!from || !to) {
        return nullptr;
    }

    // If types are equal, no conversion needed
    if (from->equals(*to)) {
        return nullptr;
    }

    // If conversion is possible, return the target type
    if (from->canConvertTo(*to)) {
        return to;
    }

    // No valid conversion
    return nullptr;
}
