#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast.h"
#include "error_handler.h"
#include "scope_manager.h"
#include "type.h"
#include <algorithm>
#include <climits>
#include <string>
#include <unordered_map>
#include <vector>


// SemanticAnalyzer - Visitor that walks the AST and performs semantic analysis
// Responsibilities:
// - Register all declarations in appropriate scopes
// - Detect and report redeclaration errors
// - Manage scope entry/exit for blocks, functions, etc.
// - Type checking for expressions and operations
class SemanticAnalyzer : public ASTVisitor {
private:
  ScopeManager scope_manager_;
  ErrorHandler error_handler_; // Unified error reporting
  bool in_function_scope_;     // Track if we're currently inside a function
  bool warnings_enabled_;      // Control whether warnings are emitted

  // Type tracking for expressions (maps expression pointer to its type)
  std::unordered_map<const Expression *, std::shared_ptr<Type>>
      expression_types_;

  // USER STORY #13: Struct type registry (maps struct name to Type)
  std::unordered_map<std::string, std::shared_ptr<Type>> struct_types_;

  // Current function context (for return type checking)
  std::string current_function_name_;
  std::shared_ptr<Type> current_function_return_type_;
  bool current_function_has_return_; // Track if we've seen a return statement

  // Helper to add an error
  void add_error(const std::string &message, const SourceLocation &location);

  // Helper to add a warning
  void add_warning(const std::string &message, const SourceLocation &location);

  // Helper to register a symbol and check for redeclaration
  bool register_symbol(const Symbol &symbol, const SourceLocation &location);

  // Helper to get the type of an expression
  std::shared_ptr<Type> get_expression_type(const Expression *expr) const;

  // Helper to set the type of an expression
  void set_expression_type(const Expression *expr, std::shared_ptr<Type> type);

  // Helper to check if an expression is an lvalue
  bool is_lvalue(const Expression *expr) const;

  // Helper to find similar identifier names (for suggestions)
  std::string find_similar_identifier(const std::string &name) const;

  // Helper to calculate Levenshtein distance between two strings
  static int levenshtein_distance(const std::string &s1, const std::string &s2);

  // USER STORY #11: Implicit Type Conversion Helpers

  // Apply integer promotion (char/short -> int)
  std::shared_ptr<Type>
  apply_integer_promotion(std::shared_ptr<Type> type) const;

  // Get common type for arithmetic operation (usual arithmetic conversions)
  std::shared_ptr<Type> get_common_type(std::shared_ptr<Type> left,
                                        std::shared_ptr<Type> right) const;

  // Check if conversion is needed and return target type (nullptr if no
  // conversion needed)
  std::shared_ptr<Type> needs_conversion(std::shared_ptr<Type> from,
                                         std::shared_ptr<Type> to) const;

  // Register built-in C standard library functions
  void register_builtin_functions();
  void register_builtin_function(const std::string &name,
                                 const std::string &return_type,
                                 const std::vector<std::string> &param_types,
                                 bool is_variadic);

public:
  SemanticAnalyzer();

  // Analyze a program (list of declarations)
  void analyze_program(
      const std::vector<std::unique_ptr<Declaration>> &declarations);

  // Access to error handler
  ErrorHandler &getErrorHandler() { return error_handler_; }
  const ErrorHandler &getErrorHandler() const { return error_handler_; }

  // Check if analysis found any errors
  bool has_errors() const { return error_handler_.has_errors(); }

  // Check if analysis found any warnings
  bool has_warnings() const { return error_handler_.has_warnings(); }

  // Enable/disable warnings
  void set_warnings_enabled(bool enabled) { warnings_enabled_ = enabled; }
  bool are_warnings_enabled() const { return warnings_enabled_; }

  // Expression visitors
  void visit(BinaryExpr &node) override;
  void visit(UnaryExpr &node) override;
  void visit(LiteralExpr &node) override;
  void visit(IdentifierExpr &node) override;
  void visit(CallExpr &node) override;
  void visit(AssignmentExpr &node) override;
  void visit(ArrayAccessExpr &node) override;
  void visit(MemberAccessExpr &node) override;
  void visit(TypeCastExpr &node) override;

  // Statement visitors
  void visit(IfStmt &node) override;
  void visit(WhileStmt &node) override;
  void visit(ForStmt &node) override;
  void visit(ReturnStmt &node) override;
  void visit(CompoundStmt &node) override;
  void visit(ExpressionStmt &node) override;
  void visit(DeclStmt &node) override;
  void visit(BreakStmt &node) override;
  void visit(ContinueStmt &node) override;

  // Declaration visitors - these are the main ones for registration
  void visit(VarDecl &node) override;
  void visit(TypeDecl &node) override;
  void visit(StructDecl &node) override;
  void visit(FunctionDecl &node) override;
  void visit(ParameterDecl &node) override;
};

#endif // SEMANTIC_ANALYZER_H
