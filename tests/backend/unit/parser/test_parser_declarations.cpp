/**
 * ==============================================================================
 * Parser Declaration Parsing Tests
 * ==============================================================================
 *
 * Module Under Test: Parser (parser.h, parser.cpp)
 *
 * Purpose:
 *   Tests parsing of top-level declarations:
 *   - Variable declarations (global and local)
 *   - Function declarations and definitions
 *   - Struct definitions
 *   - Arrays and pointers
 *   - Type specifiers and qualifiers
 *
 * Coverage:
 *   ✓ Simple variable declarations
 *   ✓ Function declarations/definitions
 *   ✓ Struct definitions
 *   ✓ Pointer and array declarations
 *   ✓ Complete programs
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "test_helpers.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"

using namespace mycc_test;

class ParserDeclarationTest : public ::testing::Test {};

// ==============================================================================
// Variable Declarations
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesSimpleGlobalVariable) {
    std::string source = "int x;";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* var_decl = dynamic_cast<VarDecl*>(program[0].get());
    ASSERT_NE(var_decl, nullptr);
    ASSERT_EQ(var_decl->getName(), "x");
    ASSERT_EQ(var_decl->getType(), "int");
}

TEST_F(ParserDeclarationTest, ParsesVariableWithInitializer) {
    std::string source = "int x = 42;";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* var_decl = dynamic_cast<VarDecl*>(program[0].get());
    ASSERT_NE(var_decl, nullptr);
    ASSERT_EQ(var_decl->getName(), "x");
    ASSERT_NE(var_decl->getInitializer(), nullptr);
}

TEST_F(ParserDeclarationTest, ParsesMultipleVariableTypes) {
    std::string source = R"(
        int a;
        float b;
        char c;
        double d;
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 4);
}

TEST_F(ParserDeclarationTest, ParsesConstVariable) {
    std::string source = "const int MAX = 100;";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesStaticVariable) {
    std::string source = "static int counter = 0;";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

// ==============================================================================
// Pointer Declarations
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesSimplePointer) {
    std::string source = "int *ptr;";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* var_decl = dynamic_cast<VarDecl*>(program[0].get());
    ASSERT_NE(var_decl, nullptr);
    ASSERT_EQ(var_decl->getName(), "ptr");
}

TEST_F(ParserDeclarationTest, ParsesPointerWithInitializer) {
    std::string source = "int *ptr = 0;";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesDoublePointer) {
    std::string source = "int **ptr;";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

// ==============================================================================
// Array Declarations
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesSimpleArray) {
    std::string source = "int arr[10];";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesArrayWithInitializer) {
    std::string source = "int arr[] = {1, 2, 3};";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesMultidimensionalArray) {
    std::string source = "int matrix[3][3];";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

// ==============================================================================
// Function Declarations
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesSimpleFunctionDeclaration) {
    std::string source = "int add(int a, int b);";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* func_decl = dynamic_cast<FunctionDecl*>(program[0].get());
    ASSERT_NE(func_decl, nullptr);
    ASSERT_EQ(func_decl->getName(), "add");
    ASSERT_EQ(func_decl->getReturnType(), "int");
}

TEST_F(ParserDeclarationTest, ParsesFunctionWithNoParameters) {
    std::string source = "void foo();";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesFunctionWithMultipleParameters) {
    std::string source = "int sum(int a, int b, int c);";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* func_decl = dynamic_cast<FunctionDecl*>(program[0].get());
    ASSERT_NE(func_decl, nullptr);
    ASSERT_EQ(func_decl->getParameters().size(), 3);
}

// ==============================================================================
// Function Definitions
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesSimpleFunctionDefinition) {
    std::string source = R"(
        int add(int a, int b) {
            return a + b;
        }
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* func_decl = dynamic_cast<FunctionDecl*>(program[0].get());
    ASSERT_NE(func_decl, nullptr);
    ASSERT_NE(func_decl->getBody(), nullptr) << "Function should have body";
}

TEST_F(ParserDeclarationTest, ParsesVoidFunction) {
    std::string source = R"(
        void print_hello() {
            printf("Hello\n");
        }
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesMainFunction) {
    std::string source = R"(
        int main() {
            return 0;
        }
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 1);
    auto* func_decl = dynamic_cast<FunctionDecl*>(program[0].get());
    ASSERT_NE(func_decl, nullptr);
    ASSERT_EQ(func_decl->getName(), "main");
}

// ==============================================================================
// Struct Definitions
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesSimpleStructDefinition) {
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesEmptyStruct) {
    std::string source = "struct Empty {};";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesStructWithMultipleFields) {
    std::string source = R"(
        struct Person {
            char name[50];
            int age;
            float height;
        };
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesStructVariable) {
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };
        struct Point p;
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_GE(program.size(), 2);
}

// ==============================================================================
// Complex Declarations
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesPointerToFunction) {
    std::string source = "int (*func_ptr)(int, int);";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesArrayOfPointers) {
    std::string source = "int *arr[10];";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

TEST_F(ParserDeclarationTest, ParsesPointerToArray) {
    std::string source = "int (*ptr)[10];";
    auto program = parse_program_without_errors(source);

    ASSERT_FALSE(program.empty());
}

// ==============================================================================
// Complete Programs
// ==============================================================================

TEST_F(ParserDeclarationTest, ParsesCompleteProgram) {
    std::string source = R"(
        int global_var = 42;

        int add(int a, int b) {
            return a + b;
        }

        int main() {
            int x = 10;
            int y = 20;
            int result = add(x, y);
            return result;
        }
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_EQ(program.size(), 3) << "Should have global var, add function, main function";
}

TEST_F(ParserDeclarationTest, ParsesProgramWithStructsAndFunctions) {
    std::string source = R"(
        struct Point {
            int x;
            int y;
        };

        int distance(struct Point p1, struct Point p2) {
            int dx = p2.x - p1.x;
            int dy = p2.y - p1.y;
            return dx * dx + dy * dy;
        }

        int main() {
            struct Point p1;
            struct Point p2;
            int dist = distance(p1, p2);
            return 0;
        }
    )";
    auto program = parse_program_without_errors(source);

    ASSERT_GE(program.size(), 3);
}
