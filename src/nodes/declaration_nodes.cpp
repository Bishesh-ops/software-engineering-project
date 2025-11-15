#include "../../include/ast.h"

// ============================================================================
// Declaration Node Implementation Documentation
// ============================================================================

/*
 * Declaration Nodes
 * -----------------
 * Declaration nodes represent code constructs that introduce new identifiers
 * into the program: variables, types, structs, functions, and parameters.
 * Declarations are processed during semantic analysis to build the symbol table.
 *
 * Declaration Node Types:
 * -----------------------
 *
 * 1. VarDecl - Variable Declaration
 *    Syntax: type name [= initializer];
 *    Components:
 *      - name: Variable identifier
 *      - type: Type string (e.g., "int", "char*", "struct Point")
 *      - initializer: Optional initial value expression
 *    Examples:
 *      "int x;" -> VarDecl("x", "int", nullptr)
 *      "int y = 42;" -> VarDecl("y", "int", LiteralExpr("42"))
 *      "char *str = "hello";" -> VarDecl("str", "char*", LiteralExpr("hello"))
 *
 *    Scope: Can be local (function scope) or global (file scope)
 *    Semantic: Type must exist, initializer must match type
 *
 * 2. TypeDecl - Type Alias Declaration (typedef)
 *    Syntax: typedef underlyingType name;
 *    Components:
 *      - name: New type name
 *      - underlyingType: Base type being aliased
 *    Examples:
 *      "typedef int Int32;" -> TypeDecl("Int32", "int")
 *      "typedef char* String;" -> TypeDecl("String", "char*")
 *
 *    Purpose: Creates type aliases for readability and portability
 *    Semantic: Underlying type must exist
 *
 * 3. StructDecl - Structure Declaration
 *    Syntax: struct name { field1; field2; ... };
 *    Components:
 *      - name: Struct tag/name
 *      - fields: Vector of VarDecl representing struct members
 *    Example: "struct Point { int x; int y; };"
 *      StructDecl("Point", [
 *        VarDecl("x", "int", nullptr),
 *        VarDecl("y", "int", nullptr)
 *      ])
 *
 *    Purpose: Defines composite data types
 *    Semantic: Field names must be unique within struct, field types must exist
 *    Memory: Fields laid out sequentially (with padding for alignment)
 *
 * 4. FunctionDecl - Function Declaration/Definition
 *    Syntax: returnType name(param1, param2, ...) [body]
 *    Components:
 *      - name: Function identifier
 *      - returnType: Return type string
 *      - parameters: Vector of ParameterDecl
 *      - body: CompoundStmt (nullptr for forward declarations)
 *    Examples:
 *      Forward: "int add(int a, int b);" -> body is nullptr
 *      Definition: "int add(int a, int b) { return a + b; }" -> body exists
 *
 *      FunctionDecl("add", "int", [
 *        ParameterDecl("a", "int"),
 *        ParameterDecl("b", "int")
 *      ], CompoundStmt([ReturnStmt(BinaryExpr(...))]))
 *
 *    Special case: "int main()" - program entry point
 *    Semantic: Return type and parameter types must exist
 *              Body must have return statement if non-void
 *              No duplicate function names (without overloading)
 *
 * 5. ParameterDecl - Function Parameter Declaration
 *    Syntax: type name
 *    Components:
 *      - name: Parameter identifier
 *      - type: Parameter type string
 *    Examples:
 *      "int x" -> ParameterDecl("x", "int")
 *      "char* str" -> ParameterDecl("str", "char*")
 *
 *    Purpose: Declares formal parameters for functions
 *    Scope: Parameters are local to function body
 *    Semantic: Type must exist, names must be unique within parameter list
 *
 * Symbol Table Integration:
 * --------------------------
 * During semantic analysis, declarations populate the symbol table:
 *   - VarDecl: Add variable to current scope
 *   - TypeDecl: Add type alias to type table
 *   - StructDecl: Add struct type to type table with field info
 *   - FunctionDecl: Add function to global scope with signature
 *   - ParameterDecl: Add parameters to function's local scope
 *
 * Declaration vs Definition:
 * --------------------------
 * - Declaration: Introduces a name (may appear multiple times)
 *   Example: "int foo();" (forward declaration)
 *
 * - Definition: Provides implementation (appears once)
 *   Example: "int foo() { return 42; }" (definition)
 *
 * Some nodes can be both:
 *   - VarDecl with initializer is both declaration and definition
 *   - FunctionDecl with body is both declaration and definition
 *
 * Storage and Linkage:
 * --------------------
 * (Future consideration for semantic analysis)
 * - Global variables: static storage, external linkage
 * - Local variables: automatic storage, no linkage
 * - Static variables: static storage, internal linkage
 * - Function parameters: automatic storage, no linkage
 *
 * Type System:
 * ------------
 * The type strings stored in declarations should be resolved during
 * semantic analysis:
 *   - Basic types: int, char, float, void
 *   - Pointer types: int*, char**, etc.
 *   - Struct types: struct StructName
 *   - Array types: int[10] (future)
 *   - Function types: int(int, int) (future)
 */

// Note: All declaration class definitions are inline in ast.h.
// This file provides documentation and can contain helper functions if needed.
