#include "../../include/ast.h"

// ============================================================================
// AST Visitor Pattern Implementation
// ============================================================================

/*
 * This file contains documentation for the AST Visitor pattern.
 *
 * The Visitor Pattern
 * -------------------
 * The visitor pattern allows you to define new operations on AST nodes without
 * modifying the node classes themselves. This is crucial for a compiler because
 * different compilation phases need to perform different operations on the same
 * AST structure.
 *
 * How It Works:
 * 1. Each AST node has an accept(ASTVisitor&) method
 * 2. The accept method calls visitor.visit(*this)
 * 3. The visitor has a visit() method overload for each node type
 * 4. This achieves double dispatch - the right visit method is called
 *    based on both the visitor type AND the node type
 *
 * Example Usage:
 * --------------
 *
 * class MyVisitor : public ASTVisitor {
 * public:
 *     void visit(BinaryExpr &node) override {
 *         // Process binary expression
 *         node.getLeft()->accept(*this);  // Visit left subtree
 *         node.getRight()->accept(*this); // Visit right subtree
 *     }
 *
 *     void visit(IfStmt &node) override {
 *         // Process if statement
 *         node.getCondition()->accept(*this);
 *         node.getThenBranch()->accept(*this);
 *         if (node.getElseBranch()) {
 *             node.getElseBranch()->accept(*this);
 *         }
 *     }
 *
 *     // ... implement other visit methods
 * };
 *
 * // To use the visitor:
 * MyVisitor visitor;
 * astRoot->accept(visitor);
 *
 *
 * Common Visitor Uses in Compilers:
 * ----------------------------------
 * 1. ASTPrinter: Pretty-print the AST structure (debugging)
 * 2. SemanticAnalyzer: Type checking, symbol resolution
 * 3. IRGenerator: Convert AST to intermediate representation
 * 4. Optimizer: Perform AST-level optimizations
 * 5. CodeGenerator: Generate target code
 *
 * Benefits:
 * ---------
 * - Separation of concerns: Each visitor handles one phase of compilation
 * - Open/Closed Principle: Add new operations without modifying AST nodes
 * - Type safety: Compiler ensures all node types are handled
 * - Flexibility: Easy to create specialized visitors for different tasks
 */

// Note: The ASTVisitor class is defined as a pure interface in ast.h.
// Concrete visitor implementations (like ASTPrinter) will be in separate files.
