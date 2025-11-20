#include "../../include/ast.h"

// ============================================================================
// Expression Node Implementation Documentation
// ============================================================================

/*
 * Expression Nodes
 * ----------------
 * Expression nodes represent code constructs that evaluate to a value.
 * In C, expressions can appear in various contexts: assignments, conditions,
 * function arguments, array indices, etc.
 *
 * Expression Node Types:
 * ----------------------
 *
 * 1. BinaryExpr - Binary operations (two operands)
 *    Examples: a + b, x * y, i < 10, p == nullptr
 *    Operators: arithmetic (+, -, *, /, %), comparison (==, !=, <, >, <=, >=),
 *               logical (&&, ||), bitwise (&, |, ^, <<, >>)
 *    Structure: left operand, operator, right operand
 *
 * 2. UnaryExpr - Unary operations (one operand)
 *    Examples: -x, !flag, *ptr, &var, ++i, i--
 *    Operators: arithmetic (-), logical (!), pointer (*, &), increment (++, --)
 *    Note: Stores whether operator is prefix (++x) or postfix (x++)
 *
 * 3. LiteralExpr - Constant values
 *    Examples: 42 (int), 3.14 (float), "hello" (string), 'c' (char), true (bool)
 *    Stores: value as string and type (INTEGER, FLOAT, STRING, CHAR, BOOLEAN)
 *
 * 4. IdentifierExpr - Variable or function references
 *    Examples: x, myFunction, arrayName
 *    Stores: name of the identifier
 *    Note: Symbol resolution happens during semantic analysis
 *
 * 5. CallExpr - Function calls
 *    Examples: foo(), printf("hello"), sum(a, b, c)
 *    Stores: callee expression (usually IdentifierExpr) + argument list
 *    Note: Arguments are evaluated left-to-right in C
 *
 * Usage in Parser:
 * ----------------
 * The parser creates these nodes while parsing expressions, typically using
 * operator precedence parsing or recursive descent:
 *
 * Example - Parsing "a + b * c":
 *   - Creates BinaryExpr with operator "*"
 *     - Left: IdentifierExpr("b")
 *     - Right: IdentifierExpr("c")
 *   - Creates outer BinaryExpr with operator "+"
 *     - Left: IdentifierExpr("a")
 *     - Right: (the multiplication BinaryExpr)
 *
 * Example - Parsing "foo(x, y + 2)":
 *   - Creates CallExpr
 *     - Callee: IdentifierExpr("foo")
 *     - Arguments:
 *       [0] IdentifierExpr("x")
 *       [1] BinaryExpr(IdentifierExpr("y"), "+", LiteralExpr("2"))
 *
 * Memory Management:
 * ------------------
 * All child expressions are owned via std::unique_ptr, ensuring automatic
 * cleanup when the parent expression is destroyed. No manual memory management
 * is required.
 *
 * Traversal:
 * ----------
 * Use the visitor pattern to traverse and process expressions:
 *   - Call expr->accept(visitor)
 *   - The visitor's visit() method is called with the specific node type
 *   - Recursively visit child expressions as needed
 */

// Note: All expression class definitions are inline in ast.h.
// This file provides documentation and can contain helper functions if needed.
