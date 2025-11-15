#include "../../include/ast.h"

// ============================================================================
// AST Node Implementation
// ============================================================================

/*
 * This file contains the implementation details for the base AST node class.
 *
 * The AST (Abstract Syntax Tree) node hierarchy represents the parsed structure
 * of source code. Each node stores its source location (line/column) for error
 * reporting and implements the visitor pattern for tree traversal.
 *
 * Design Pattern: Visitor Pattern
 * --------------------------------
 * The AST uses the visitor pattern to allow different operations (semantic
 * analysis, code generation, optimization, etc.) to be performed on the tree
 * without modifying the node classes themselves.
 *
 * Key Features:
 * - All nodes inherit from ASTNode base class
 * - Each node stores SourceLocation for error reporting
 * - Virtual accept() method enables visitor pattern
 * - Smart pointers (unique_ptr) manage memory automatically
 *
 * Node Categories:
 * ----------------
 * 1. Expression: Nodes that evaluate to a value (literals, operations, calls)
 * 2. Statement: Nodes that perform actions (if, while, return, etc.)
 * 3. Declaration: Nodes that introduce new entities (variables, functions, types)
 *
 * The actual class definitions are in ast.h (header-only for template support).
 */

// Note: All AST node classes are defined inline in ast.h for better
// compiler optimization and template instantiation. This file serves as
// documentation and can contain future helper functions if needed.
