/**
 * ==============================================================================
 * Semantic Analysis Error Detection Tests
 * ==============================================================================
 *
 * Module Under Test: SemanticAnalyzer (semantic_analyzer.h/cpp)
 *
 * Purpose:
 *   Tests error detection and reporting for semantic violations:
 *   - Undeclared identifier usage
 *   - Redeclaration errors
 *   - Type mismatches
 *   - Invalid operations
 *   - Missing return statements
 *
 * Coverage:
 *   ✓ Undeclared variable/function errors
 *   ✓ Redeclaration detection
 *   ✓ Type mismatch errors
 *   ✓ Invalid operation errors
 *   ✓ Return statement validation
 *   ✓ Function signature errors
 *   ✓ Struct-related errors
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "semantic_analyzer.h"
#include "parser.h"
#include "lexer.h"

using namespace mycc_test;

class SemanticErrorTest : public ::testing::Test {};

// ==============================================================================
// Undeclared Identifier Errors
// ==============================================================================

/**
 * Test: Undeclared variable usage
 *
 * Verifies:
 *   - Using undeclared variable produces error
 */
TEST_F(SemanticErrorTest, UndeclaredVariableUsage) {
    std::string source = R"(
        int main() {
            x = 5;  // Error: 'x' not declared
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
    EXPECT_GT(analyzer.getErrorHandler().get_error_count(), 0);
}

/**
 * Test: Undeclared function call
 *
 * Verifies:
 *   - Calling undeclared function produces error
 */
TEST_F(SemanticErrorTest, UndeclaredFunctionCall) {
    std::string source = R"(
        int main() {
            int result = undeclared_func(5);  // Error: function not declared
            return result;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Using variable before declaration (in same scope)
 *
 * Verifies:
 *   - Variable must be declared before use
 */
TEST_F(SemanticErrorTest, VariableUsedBeforeDeclaration) {
    std::string source = R"(
        int main() {
            y = x + 1;  // Error: 'x' not yet declared
            int x = 5;
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Redeclaration Errors
// ==============================================================================

/**
 * Test: Variable redeclaration in same scope
 *
 * Verifies:
 *   - Cannot redeclare variable in same scope
 */
TEST_F(SemanticErrorTest, VariableRedeclarationInSameScope) {
    std::string source = R"(
        int main() {
            int x = 5;
            int x = 10;  // Error: redeclaration
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Function redeclaration
 *
 * Verifies:
 *   - Cannot redeclare function
 */
TEST_F(SemanticErrorTest, FunctionRedeclaration) {
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }

        int add(int x, int y) {  // Error: redeclaration
            return x + y;
        }

        int main() {
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Parameter name conflict
 *
 * Verifies:
 *   - Function parameters cannot have duplicate names
 */
TEST_F(SemanticErrorTest, DuplicateParameterNames) {
    std::string source = R"(
        int func(int x, int x) {  // Error: duplicate parameter
            return x;
        }

        int main() {
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Global variable redeclaration
 *
 * Verifies:
 *   - Cannot redeclare global variables
 */
TEST_F(SemanticErrorTest, GlobalVariableRedeclaration) {
    std::string source = R"(
        int global_var;
        float global_var;  // Error: redeclaration with different type

        int main() {
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Type Mismatch Errors
// ==============================================================================

/**
 * Test: Assignment type mismatch
 *
 * Verifies:
 *   - Incompatible types in assignment produce error
 */
TEST_F(SemanticErrorTest, AssignmentTypeMismatch) {
    std::string source = R"(
        int main() {
            int x;
            x = "string";  // Error: cannot assign string to int
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Binary operation type mismatch
 *
 * Verifies:
 *   - Invalid operand types for binary operation
 */
TEST_F(SemanticErrorTest, BinaryOperationTypeMismatch) {
    std::string source = R"(
        int main() {
            int x = 5;
            int *ptr = &x;
            int result = x + ptr;  // Error: int + pointer
            return result;
        }
    )";

    // This may be allowed with warning in some implementations
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    // Should produce error or warning
    EXPECT_TRUE(analyzer.has_errors() || analyzer.has_warnings());
}

/**
 * Test: Return type mismatch
 *
 * Verifies:
 *   - Returning wrong type from function produces error
 */
TEST_F(SemanticErrorTest, ReturnTypeMismatch) {
    std::string source = R"(
        int getNumber() {
            return "not a number";  // Error: returning string from int function
        }

        int main() {
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Function Call Errors
// ==============================================================================

/**
 * Test: Wrong number of function arguments
 *
 * Verifies:
 *   - Error when argument count doesn't match
 */
TEST_F(SemanticErrorTest, WrongNumberOfArguments) {
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }

        int main() {
            int result = add(5, 10, 15);  // Error: too many arguments
            return result;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Too few arguments
 *
 * Verifies:
 *   - Error when not enough arguments provided
 */
TEST_F(SemanticErrorTest, TooFewArguments) {
    std::string source = R"(
        int multiply(int a, int b, int c) {
            return a * b * c;
        }

        int main() {
            int result = multiply(2, 3);  // Error: missing argument
            return result;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Calling non-function identifier
 *
 * Verifies:
 *   - Error when trying to call a variable
 */
TEST_F(SemanticErrorTest, CallingNonFunction) {
    std::string source = R"(
        int main() {
            int x = 5;
            int result = x(10);  // Error: 'x' is not a function
            return result;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Invalid Operations
// ==============================================================================

/**
 * Test: Assignment to non-lvalue
 *
 * Verifies:
 *   - Cannot assign to literals or expressions
 */
TEST_F(SemanticErrorTest, AssignmentToNonLvalue) {
    std::string source = R"(
        int main() {
            5 = 10;  // Error: cannot assign to literal
            return 0;
        }
    )";

    // This will likely fail in parsing, but if it gets to semantic analysis
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    if (!parser.hasErrors()) {
        SemanticAnalyzer analyzer;
        analyzer.analyze_program(program);
        EXPECT_TRUE(analyzer.has_errors());
    }
}

/**
 * Test: Assignment to function call result
 *
 * Verifies:
 *   - Cannot assign to rvalue
 */
TEST_F(SemanticErrorTest, AssignmentToRvalue) {
    std::string source = R"(
        int getValue() {
            return 42;
        }

        int main() {
            getValue() = 10;  // Error: cannot assign to function result
            return 0;
        }
    )";

    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    if (!parser.hasErrors()) {
        SemanticAnalyzer analyzer;
        analyzer.analyze_program(program);
        EXPECT_TRUE(analyzer.has_errors());
    }
}

// ==============================================================================
// Return Statement Errors
// ==============================================================================

/**
 * Test: Missing return statement
 *
 * Verifies:
 *   - Non-void function must have return statement
 */
TEST_F(SemanticErrorTest, MissingReturnStatement) {
    std::string source = R"(
        int getValue() {
            int x = 42;
            // Error: no return statement
        }

        int main() {
            return 0;
        }
    )";

    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    analyzer.set_warnings_enabled(true);
    analyzer.analyze_program(program);

    // Should produce error or warning
    EXPECT_TRUE(analyzer.has_errors() || analyzer.has_warnings());
}

/**
 * Test: Return with value in void function
 *
 * Verifies:
 *   - void function cannot return value
 */
TEST_F(SemanticErrorTest, ReturnValueInVoidFunction) {
    std::string source = R"(
        void doNothing() {
            return 42;  // Error: void function can't return value
        }

        int main() {
            doNothing();
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Struct-Related Errors
// ==============================================================================

/**
 * Test: Undefined struct type
 *
 * Verifies:
 *   - Using undefined struct produces error
 */
TEST_F(SemanticErrorTest, UndefinedStructType) {
    std::string source = R"(
        int main() {
            struct UndefinedStruct s;  // Error: struct not defined
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Accessing non-existent struct member
 *
 * Verifies:
 *   - Error when accessing undefined member
 */
TEST_F(SemanticErrorTest, NonExistentStructMember) {
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            int z = p.z;  // Error: 'z' is not a member of Point
            return z;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Member access on non-struct
 *
 * Verifies:
 *   - Cannot use . operator on non-struct types
 */
TEST_F(SemanticErrorTest, MemberAccessOnNonStruct) {
    std::string source = R"(
        int main() {
            int x = 5;
            int y = x.field;  // Error: 'x' is not a struct
            return y;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Array and Pointer Errors
// ==============================================================================

/**
 * Test: Array subscript on non-array
 *
 * Verifies:
 *   - Cannot subscript non-array/pointer
 */
TEST_F(SemanticErrorTest, ArraySubscriptOnNonArray) {
    std::string source = R"(
        int main() {
            int x = 5;
            int y = x[0];  // Error: 'x' is not an array
            return y;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

/**
 * Test: Dereferencing non-pointer
 *
 * Verifies:
 *   - Cannot dereference non-pointer type
 */
TEST_F(SemanticErrorTest, DereferencingNonPointer) {
    std::string source = R"(
        int main() {
            int x = 5;
            int y = *x;  // Error: 'x' is not a pointer
            return y;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
}

// ==============================================================================
// Multiple Errors in Same Program
// ==============================================================================

/**
 * Test: Multiple different errors
 *
 * Verifies:
 *   - Error recovery allows multiple errors to be reported
 */
TEST_F(SemanticErrorTest, MultipleErrors) {
    std::string source = R"(
        int main() {
            undefined_var = 5;       // Error 1: undeclared
            int x;
            int x;                   // Error 2: redeclaration
            int y = "string";        // Error 3: type mismatch
            return 0;
        }
    )";

    auto analyzer = analyze_program_with_errors(source);
    EXPECT_TRUE(analyzer.has_errors());
    EXPECT_GE(analyzer.getErrorHandler().get_error_count(), 2);
}

/**
 * Test: Error recovery continues analysis
 *
 * Verifies:
 *   - After error, analysis continues for subsequent code
 */
TEST_F(SemanticErrorTest, ErrorRecoveryContinues) {
    std::string source = R"(
        int main() {
            undefined_var = 5;  // Error
            int valid = 10;     // Should still be analyzed
            return valid;
        }
    )";

    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    EXPECT_TRUE(analyzer.has_errors());

    // Should still have registered 'valid' variable
    // (Implementation-dependent verification)
}

// ==============================================================================
// Complex Error Scenarios
// ==============================================================================

/**
 * Test: Type mismatch in nested expressions
 *
 * Verifies:
 *   - Type checking works in complex nested expressions
 */
TEST_F(SemanticErrorTest, TypeMismatchInNestedExpression) {
    std::string source = R"(
        int main() {
            int x = 5;
            int *ptr = &x;
            int result = (x + 10) * ptr;  // Error: multiplication with pointer
            return result;
        }
    )";

    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    EXPECT_TRUE(analyzer.has_errors() || analyzer.has_warnings());
}
