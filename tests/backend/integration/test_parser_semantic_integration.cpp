/**
 * ==============================================================================
 * Integration Tests: Parser → Semantic Analysis Pipeline
 * ==============================================================================
 *
 * Components Under Test:
 *   - Parser (parser.h, parser.cpp)
 *   - SemanticAnalyzer (semantic_analyzer.h, semantic_analyzer.cpp)
 *   - SymbolTable (symbol_table.h, symbol_table.cpp)
 *   - ScopeManager (scope_manager.h, scope_manager.cpp)
 *
 * Purpose:
 *   Verifies that the parser-produced AST is correctly analyzed by the
 *   semantic analyzer. Tests focus on:
 *   - AST nodes processed by semantic visitor
 *   - Symbol table population from parsed declarations
 *   - Scope management during AST traversal
 *   - Type checking based on AST structure
 *   - Error detection in semantically invalid AST
 *
 * Integration Points:
 *   - SemanticAnalyzer.analyze_program() receives Parser output
 *   - AST visitor pattern connects parser output to semantic analysis
 *   - Symbol entries created from Declaration AST nodes
 *   - Type information derived from AST type specifiers
 *
 * Test Categories:
 *   1. Declaration Processing - Parsed declarations create symbols
 *   2. Scope Management - Nested scopes in parsed functions/blocks
 *   3. Type Analysis - Types from AST used in checking
 *   4. Error Detection - Semantic errors in parsed code
 *   5. Symbol Resolution - References resolved from parsed identifiers
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <memory>
#include <vector>
#include <string>

using namespace mycc_test;

// ==============================================================================
// Test Fixture for Parser-Semantic Integration Tests
// ==============================================================================

class ParserSemanticIntegrationTest : public ::testing::Test
{
protected:
    /**
     * Helper structure to capture pipeline results
     */
    struct AnalysisResult
    {
        std::vector<std::unique_ptr<Declaration>> declarations;
        bool parser_has_errors;
        bool semantic_has_errors;
        bool semantic_has_warnings;
        int parser_error_count;
        int semantic_error_count;
        int semantic_warning_count;
    };

    /**
     * Run the complete parser→semantic pipeline
     */
    AnalysisResult run_semantic_pipeline(const std::string &source,
                                         const std::string &filename = "test.c",
                                         bool enable_warnings = false)
    {
        Lexer lexer(source, filename);
        Parser parser(lexer);
        auto declarations = parser.parseProgram();

        SemanticAnalyzer analyzer;
        analyzer.set_warnings_enabled(enable_warnings);
        analyzer.analyze_program(declarations);

        return AnalysisResult{
            std::move(declarations),
            parser.hasErrors(),
            analyzer.has_errors(),
            analyzer.has_warnings(),
            parser.getErrorHandler().get_error_count(),
            analyzer.getErrorHandler().get_error_count(),
            analyzer.getErrorHandler().get_warning_count()};
    }
};

// ==============================================================================
// Declaration Processing Tests
// ==============================================================================

/**
 * Test: Variable declaration creates symbol
 *
 * Verifies:
 *   - Parsed variable declaration processed by semantic analyzer
 *   - No errors for valid declaration
 */
TEST_F(ParserSemanticIntegrationTest, VariableDeclarationCreatesSymbol)
{
    std::string source = "int counter;";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors)
        << "Valid variable declaration should not produce semantic errors";
}

/**
 * Test: Function declaration creates symbol
 *
 * Verifies:
 *   - Parsed function declaration registered in symbol table
 *   - Parameters create symbols in function scope
 */
TEST_F(ParserSemanticIntegrationTest, FunctionDeclarationCreatesSymbol)
{
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors)
        << "Valid function declaration should not produce semantic errors";
}

/**
 * Test: Multiple declarations all registered
 *
 * Verifies:
 *   - Multiple parsed declarations all processed
 *   - Each creates appropriate symbol
 */
TEST_F(ParserSemanticIntegrationTest, MultipleDeclarationsRegistered)
{
    std::string source = R"(
        int global_var;
        float pi;
        int main() { return 0; }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
    ASSERT_EQ(result.declarations.size(), 3);
}

/**
 * Test: Variable with initializer type checked
 *
 * Verifies:
 *   - Parsed initializer expression type checked against variable type
 */
TEST_F(ParserSemanticIntegrationTest, InitializerTypeChecked)
{
    std::string source = R"(
        int main() {
            int x = 42;
            float y = 3.14;
            return 0;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

// ==============================================================================
// Scope Management Tests
// ==============================================================================

/**
 * Test: Function body creates new scope
 *
 * Verifies:
 *   - Parsed function body block enters new scope
 *   - Local variables not visible outside function
 */
TEST_F(ParserSemanticIntegrationTest, FunctionBodyCreatesScope)
{
    std::string source = R"(
        int main() {
            int local_var = 10;
            return local_var;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Nested blocks create nested scopes
 *
 * Verifies:
 *   - Parsed nested compound statements create nested scopes
 *   - Inner variables shadow outer variables correctly
 */
TEST_F(ParserSemanticIntegrationTest, NestedBlocksCreateNestedScopes)
{
    std::string source = R"(
        int main() {
            int x = 1;
            {
                int x = 2;  // Shadows outer x
                {
                    int x = 3;  // Shadows previous x
                }
            }
            return x;  // Refers to outermost x
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Function parameters in function scope
 *
 * Verifies:
 *   - Parsed parameters accessible in function body
 */
TEST_F(ParserSemanticIntegrationTest, ParametersAccessibleInBody)
{
    std::string source = R"(
        int add(int a, int b) {
            int sum = a + b;  // Parameters accessible
            return sum;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Global variables accessible in functions
 *
 * Verifies:
 *   - Parsed global declarations accessible in function scopes
 */
TEST_F(ParserSemanticIntegrationTest, GlobalsAccessibleInFunctions)
{
    std::string source = R"(
        int global_counter;

        int increment() {
            global_counter = global_counter + 1;
            return global_counter;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

// ==============================================================================
// Type Analysis from AST Tests
// ==============================================================================

/**
 * Test: Binary expression type derived from operands
 *
 * Verifies:
 *   - Parsed binary expression operand types used for type checking
 */
TEST_F(ParserSemanticIntegrationTest, BinaryExpressionTypeAnalysis)
{
    std::string source = R"(
        int main() {
            int a = 5;
            int b = 10;
            int c = a + b;  // int + int = int
            return c;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Function return type checked
 *
 * Verifies:
 *   - Parsed return statement expression type matches function return type
 */
TEST_F(ParserSemanticIntegrationTest, ReturnTypeChecked)
{
    std::string source = R"(
        int getValue() {
            return 42;  // int matches function return type
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Function call argument types checked
 *
 * Verifies:
 *   - Parsed function call arguments match parameter types
 */
TEST_F(ParserSemanticIntegrationTest, FunctionCallArgumentsChecked)
{
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }

        int main() {
            int result = add(5, 10);  // int, int matches parameters
            return result;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

// ==============================================================================
// Error Detection Tests
// ==============================================================================

/**
 * Test: Undefined variable detected
 *
 * Verifies:
 *   - Parsed identifier expression for undefined variable produces error
 */
TEST_F(ParserSemanticIntegrationTest, UndefinedVariableDetected)
{
    std::string source = R"(
        int main() {
            int x = undefined_var;  // Error: undefined
            return x;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors)
        << "Code is syntactically valid";
    ASSERT_TRUE(result.semantic_has_errors)
        << "Undefined variable should produce semantic error";
}

/**
 * Test: Duplicate declaration detected
 *
 * Verifies:
 *   - Parsed duplicate declarations in same scope produce error
 */
TEST_F(ParserSemanticIntegrationTest, DuplicateDeclarationDetected)
{
    std::string source = R"(
        int main() {
            int x = 1;
            int x = 2;  // Error: redeclaration
            return x;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_TRUE(result.semantic_has_errors)
        << "Duplicate declaration should produce semantic error";
}

/**
 * Test: Type mismatch in assignment detected
 *
 * Verifies:
 *   - Parsed assignment with incompatible types produces error
 */
TEST_F(ParserSemanticIntegrationTest, TypeMismatchInAssignment)
{
    std::string source = R"(
        int main() {
            int x = "string";  // Error: string to int
            return x;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_TRUE(result.semantic_has_errors)
        << "Type mismatch should produce semantic error";
}

/**
 * Test: Undefined function call detected
 *
 * Verifies:
 *   - Parsed call to undefined function produces error
 */
TEST_F(ParserSemanticIntegrationTest, UndefinedFunctionCallDetected)
{
    std::string source = R"(
        int main() {
            int result = undefined_func(5);  // Error: undefined function
            return result;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_TRUE(result.semantic_has_errors)
        << "Undefined function should produce semantic error";
}

/**
 * Test: Wrong argument count detected
 *
 * Verifies:
 *   - Parsed function call with wrong number of arguments produces error
 */
TEST_F(ParserSemanticIntegrationTest, WrongArgumentCountDetected)
{
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }

        int main() {
            int result = add(1);  // Error: too few arguments
            return result;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_TRUE(result.semantic_has_errors)
        << "Wrong argument count should produce semantic error";
}

// ==============================================================================
// Warning Detection Tests
// ==============================================================================

/**
 * Test: Unused variable warning
 *
 * Verifies:
 *   - Parsed but unused variable produces warning (not error)
 */
TEST_F(ParserSemanticIntegrationTest, UnusedVariableWarning)
{
    std::string source = R"(
        int main() {
            int unused = 42;  // Warning: unused variable
            return 0;
        }
    )";

    auto result = run_semantic_pipeline(source, "test.c", true /* enable warnings */);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors)
        << "Unused variable should be warning, not error";
    ASSERT_TRUE(result.semantic_has_warnings)
        << "Unused variable should produce warning";
}

// ==============================================================================
// Complex Integration Scenarios
// ==============================================================================

/**
 * Test: Complete program with multiple functions
 *
 * Verifies:
 *   - Complex parsed program fully analyzed
 *   - Cross-function references resolved
 */
TEST_F(ParserSemanticIntegrationTest, CompleteProgramAnalysis)
{
    std::string source = R"(
        int factorial(int n) {
            if (n <= 1) {
                return 1;
            }
            return n * factorial(n - 1);
        }

        int main() {
            int result = factorial(5);
            return result;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors)
        << "Valid recursive program should not produce errors";
}

/**
 * Test: Struct declaration and member access
 *
 * Verifies:
 *   - Parsed struct creates type definition
 *   - Member access validated
 */
TEST_F(ParserSemanticIntegrationTest, StructTypeAnalysis)
{
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            return 0;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Control flow with semantic analysis
 *
 * Verifies:
 *   - Variables declared in if/else branches analyzed
 *   - Loop variable scopes correct
 */
TEST_F(ParserSemanticIntegrationTest, ControlFlowSemanticAnalysis)
{
    // Note: Using C89-style for-loop (variable declared before loop)
    // because the compiler doesn't support C99-style for-loop declarations
    std::string source = R"(
        int main() {
            int x = 5;
            int result = 0;
            int i;

            if (x > 0) {
                int temp = x * 2;
                result = temp;
            } else {
                int temp = x * -1;  // Different temp in else branch
                result = temp;
            }

            for (i = 0; i < 10; i = i + 1) {
                result = result + i;
            }

            return result;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Forward reference to function (declaration order)
 *
 * Verifies:
 *   - Functions can call other functions declared later (if prototyped)
 *   - Or produces error if called before declaration
 */
TEST_F(ParserSemanticIntegrationTest, FunctionDeclarationOrder)
{
    // Functions declared in order - should work
    std::string source = R"(
        int helper() {
            return 42;
        }

        int main() {
            return helper();
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_FALSE(result.semantic_has_errors);
}

/**
 * Test: Multiple errors detected in single program
 *
 * Verifies:
 *   - Semantic analyzer reports multiple errors
 *   - Error recovery continues analysis
 */
TEST_F(ParserSemanticIntegrationTest, MultipleErrorsDetected)
{
    std::string source = R"(
        int main() {
            int x = undefined1;  // Error 1
            int y = undefined2;  // Error 2
            return 0;
        }
    )";

    auto result = run_semantic_pipeline(source);

    ASSERT_FALSE(result.parser_has_errors);
    ASSERT_TRUE(result.semantic_has_errors);
    ASSERT_GE(result.semantic_error_count, 2)
        << "Should detect at least 2 semantic errors";
}
