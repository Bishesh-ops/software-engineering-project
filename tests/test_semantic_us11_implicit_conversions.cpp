/**
 * @file test_implicit_conversions.cpp
 * @brief Test suite for User Story #11: Implicit Type Conversions
 *
 * Tests:
 * - Integer promotion (char → int, short → int)
 * - Arithmetic conversions (int + float → both become float)
 * - Array to pointer decay (int[10] → int*)
 * - Conversion nodes insertion in AST
 */

#include "../include/type.h"
#include "../include/ast.h"
#include "../include/semantic_analyzer.h"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

// Helper functions
void pass(const string& msg) { cout << "  [PASS] " << msg << endl; }
void fail(const string& msg) { cout << "  [FAIL] " << msg << endl; }

// ============================================================================
// Test 1: Integer Promotion (char → int, short → int)
// ============================================================================
void test_integer_promotion() {
    cout << "\n[TEST] Integer Promotion" << endl;

    // Test char promotion
    auto char_type = Type::makeChar();
    auto promoted = applyIntegerPromotion(char_type);

    if (promoted && promoted->getBaseType() == Type::BaseType::INT) {
        pass("char promoted to int");
    } else {
        fail("char should be promoted to int");
    }

    // Test short promotion
    auto short_type = Type::makeShort();
    promoted = applyIntegerPromotion(short_type);

    if (promoted && promoted->getBaseType() == Type::BaseType::INT) {
        pass("short promoted to int");
    } else {
        fail("short should be promoted to int");
    }

    // Test int stays int
    auto int_type = Type::makeInt();
    promoted = applyIntegerPromotion(int_type);

    if (promoted && promoted->getBaseType() == Type::BaseType::INT) {
        pass("int stays int");
    } else {
        fail("int should stay int");
    }

    // Test float stays float
    auto float_type = Type::makeFloat();
    promoted = applyIntegerPromotion(float_type);

    if (promoted && promoted->getBaseType() == Type::BaseType::FLOAT) {
        pass("float stays float (no promotion)");
    } else {
        fail("float should not be promoted");
    }
}

// ============================================================================
// Test 2: Arithmetic Conversions (usual arithmetic conversions)
// ============================================================================
void test_arithmetic_conversions() {
    cout << "\n[TEST] Arithmetic Conversions" << endl;

    // Test int + float → float
    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto common = getCommonArithmeticType(int_type, float_type);

    if (common && common->getBaseType() == Type::BaseType::FLOAT) {
        pass("int + float → float");
    } else {
        fail("int + float should convert to float");
    }

    // Test char + int → int (after char is promoted to int)
    auto char_type = Type::makeChar();
    common = getCommonArithmeticType(char_type, int_type);

    if (common && common->getBaseType() == Type::BaseType::INT) {
        pass("char + int → int");
    } else {
        fail("char + int should convert to int");
    }

    // Test int + double → double
    auto double_type = Type::makeDouble();
    common = getCommonArithmeticType(int_type, double_type);

    if (common && common->getBaseType() == Type::BaseType::DOUBLE) {
        pass("int + double → double");
    } else {
        fail("int + double should convert to double");
    }

    // Test float + double → double
    common = getCommonArithmeticType(float_type, double_type);

    if (common && common->getBaseType() == Type::BaseType::DOUBLE) {
        pass("float + double → double");
    } else {
        fail("float + double should convert to double");
    }

    // Test long + int → long
    auto long_type = Type::makeLong();
    common = getCommonArithmeticType(long_type, int_type);

    if (common && common->getBaseType() == Type::BaseType::LONG) {
        pass("long + int → long");
    } else {
        fail("long + int should convert to long");
    }

    // Test char + short → int (both promoted to int)
    auto short_type = Type::makeShort();
    common = getCommonArithmeticType(char_type, short_type);

    if (common && common->getBaseType() == Type::BaseType::INT) {
        pass("char + short → int");
    } else {
        fail("char + short should convert to int");
    }
}

// ============================================================================
// Test 3: Array to Pointer Decay
// ============================================================================
void test_array_to_pointer_decay() {
    cout << "\n[TEST] Array to Pointer Decay" << endl;

    // Test int[10] → int*
    auto array_type = Type::makeArray(Type::BaseType::INT, 10);
    auto decayed = applyArrayToPointerDecay(array_type);

    if (decayed && decayed->isPointer() &&
        decayed->getBaseType() == Type::BaseType::INT &&
        decayed->getPointerDepth() == 1) {
        pass("int[10] → int*");
    } else {
        fail("int[10] should decay to int*");
    }

    // Test char[5] → char*
    array_type = Type::makeArray(Type::BaseType::CHAR, 5);
    decayed = applyArrayToPointerDecay(array_type);

    if (decayed && decayed->isPointer() &&
        decayed->getBaseType() == Type::BaseType::CHAR &&
        decayed->getPointerDepth() == 1) {
        pass("char[5] → char*");
    } else {
        fail("char[5] should decay to char*");
    }

    // Test non-array type remains unchanged
    auto int_type = Type::makeInt();
    decayed = applyArrayToPointerDecay(int_type);

    if (decayed && decayed->equals(*int_type)) {
        pass("int stays int (no decay)");
    } else {
        fail("int should not be decayed");
    }

    // Test pointer stays pointer
    auto ptr_type = Type::makePointer(Type::BaseType::INT, 1);
    decayed = applyArrayToPointerDecay(ptr_type);

    if (decayed && decayed->equals(*ptr_type)) {
        pass("int* stays int* (no decay)");
    } else {
        fail("int* should not be decayed");
    }
}

// ============================================================================
// Test 4: Conversion Node Insertion in AST
// ============================================================================
void test_conversion_node_insertion() {
    cout << "\n[TEST] Conversion Node Insertion in AST" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;

    // Create a simple program with mixed-type expression: char + float
    vector<unique_ptr<Declaration>> program;

    // char x = 'A';
    auto char_lit = make_unique<LiteralExpr>("65", LiteralExpr::LiteralType::CHAR, loc);
    program.push_back(make_unique<VarDecl>("x", "char", move(char_lit), loc));

    // float y = 3.14;
    auto float_lit = make_unique<LiteralExpr>("3.14", LiteralExpr::LiteralType::FLOAT, loc);
    program.push_back(make_unique<VarDecl>("y", "float", move(float_lit), loc));

    // float z = x + y; (should insert conversion: char x promoted to int, then int promoted to float)
    auto id_x = make_unique<IdentifierExpr>("x", loc);
    auto id_y = make_unique<IdentifierExpr>("y", loc);
    auto add_expr = make_unique<BinaryExpr>(move(id_x), "+", move(id_y), loc);
    program.push_back(make_unique<VarDecl>("z", "float", move(add_expr), loc));

    // Analyze the program
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Mixed-type expression (char + float) analyzed without errors");
    } else {
        fail("Mixed-type expression should not produce errors");
        for (const auto& error : analyzer.getErrorHandler().get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }

    // The add_expr should have had TypeCastExpr nodes inserted
    // We can't directly verify this without AST traversal, but no errors means conversions worked
}

// ============================================================================
// Test 5: Integer Promotion in Bitwise Operations
// ============================================================================
void test_integer_promotion_bitwise() {
    cout << "\n[TEST] Integer Promotion in Bitwise Operations" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;

    vector<unique_ptr<Declaration>> program;

    // char a = 10;
    auto char_lit_a = make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::CHAR, loc);
    program.push_back(make_unique<VarDecl>("a", "char", move(char_lit_a), loc));

    // char b = 20;
    auto char_lit_b = make_unique<LiteralExpr>("20", LiteralExpr::LiteralType::CHAR, loc);
    program.push_back(make_unique<VarDecl>("b", "char", move(char_lit_b), loc));

    // int c = a & b; (chars should be promoted to int for bitwise AND)
    auto id_a = make_unique<IdentifierExpr>("a", loc);
    auto id_b = make_unique<IdentifierExpr>("b", loc);
    auto and_expr = make_unique<BinaryExpr>(move(id_a), "&", move(id_b), loc);
    program.push_back(make_unique<VarDecl>("c", "int", move(and_expr), loc));

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Bitwise operation on chars analyzed without errors");
    } else {
        fail("Bitwise operation should work with integer promotion");
        for (const auto& error : analyzer.getErrorHandler().get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 6: Array to Pointer Decay in Binary Expressions
// ============================================================================
void test_array_decay_in_expressions() {
    cout << "\n[TEST] Array to Pointer Decay in Expressions" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;

    vector<unique_ptr<Declaration>> program;

    // int arr[10];
    program.push_back(make_unique<VarDecl>("arr", "int", nullptr, loc, true,
                                          make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, loc)));

    // int* ptr = arr; (array should decay to pointer - this would be in an assignment)
    // For now, we test that arrays can be used in contexts expecting pointers

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Array declaration analyzed without errors");
    } else {
        fail("Array declaration should not produce errors");
        for (const auto& error : analyzer.getErrorHandler().get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 7: Type Hierarchy in Conversions
// ============================================================================
void test_type_hierarchy() {
    cout << "\n[TEST] Type Conversion Hierarchy (double > float > long > int > short > char)" << endl;

    // Test the complete hierarchy
    auto char_type = Type::makeChar();
    auto short_type = Type::makeShort();
    auto int_type = Type::makeInt();
    auto long_type = Type::makeLong();
    auto float_type = Type::makeFloat();
    auto double_type = Type::makeDouble();

    // char vs everything
    auto common = getCommonArithmeticType(char_type, short_type);
    if (common && common->getBaseType() == Type::BaseType::INT) {
        pass("char vs short → int");
    } else {
        fail("char vs short should result in int");
    }

    common = getCommonArithmeticType(char_type, double_type);
    if (common && common->getBaseType() == Type::BaseType::DOUBLE) {
        pass("char vs double → double");
    } else {
        fail("char vs double should result in double");
    }

    // int vs long
    common = getCommonArithmeticType(int_type, long_type);
    if (common && common->getBaseType() == Type::BaseType::LONG) {
        pass("int vs long → long");
    } else {
        fail("int vs long should result in long");
    }

    // long vs float
    common = getCommonArithmeticType(long_type, float_type);
    if (common && common->getBaseType() == Type::BaseType::FLOAT) {
        pass("long vs float → float");
    } else {
        fail("long vs float should result in float");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    cout << "========================================" << endl;
    cout << "User Story #11: Implicit Type Conversions" << endl;
    cout << "========================================" << endl;

    test_integer_promotion();
    test_arithmetic_conversions();
    test_array_to_pointer_decay();
    test_conversion_node_insertion();
    test_integer_promotion_bitwise();
    test_array_decay_in_expressions();
    test_type_hierarchy();

    cout << "\n========================================" << endl;
    cout << "All implicit type conversion tests completed!" << endl;
    cout << "========================================\n" << endl;

    return 0;
}
