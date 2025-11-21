/**
 * @file test_pointer_arithmetic.cpp
 * @brief Test suite for User Story #12: Pointer Arithmetic Validation
 *
 * Tests:
 * - ptr + int → pointer (valid, scaled by pointed-to size)
 * - ptr - int → pointer (valid, scaled by pointed-to size)
 * - int + ptr → pointer (valid for addition only)
 * - ptr - ptr → int (valid only for same type pointers)
 * - ptr * int → error
 * - ptr / int → error
 * - ptr * ptr → error
 * - ptr + ptr → error
 * - void* arithmetic → error
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

bool contains_error(const vector<SemanticError>& errors, const string& substring) {
    for (const auto& error : errors) {
        if (error.message.find(substring) != string::npos) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Test 1: Valid Pointer + Integer (ptr + int → pointer)
// ============================================================================
void test_pointer_plus_int() {
    cout << "\n[TEST] Valid: Pointer + Integer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int* ptr;
    program.push_back(make_unique<VarDecl>("ptr", "int", nullptr, loc, false, nullptr, 1));

    // int offset = 5;
    auto offset_lit = make_unique<LiteralExpr>("5", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("offset", "int", std::move(offset_lit), loc));

    // int* result = ptr + offset; (valid)
    auto id_ptr = make_unique<IdentifierExpr>("ptr", loc);
    auto id_offset = make_unique<IdentifierExpr>("offset", loc);
    auto add_expr = make_unique<BinaryExpr>(std::move(id_ptr), "+", std::move(id_offset), loc);
    program.push_back(make_unique<VarDecl>("result", "int", std::move(add_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("ptr + int is valid");
    } else {
        fail("ptr + int should be valid");
        for (const auto& error : analyzer.get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 2: Valid Integer + Pointer (int + ptr → pointer)
// ============================================================================
void test_int_plus_pointer() {
    cout << "\n[TEST] Valid: Integer + Pointer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // char* str;
    program.push_back(make_unique<VarDecl>("str", "char", nullptr, loc, false, nullptr, 1));

    // int index = 3;
    auto index_lit = make_unique<LiteralExpr>("3", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("index", "int", std::move(index_lit), loc));

    // char* result = index + str; (valid)
    auto id_index = make_unique<IdentifierExpr>("index", loc);
    auto id_str = make_unique<IdentifierExpr>("str", loc);
    auto add_expr = make_unique<BinaryExpr>(std::move(id_index), "+", std::move(id_str), loc);
    program.push_back(make_unique<VarDecl>("result", "char", std::move(add_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("int + ptr is valid");
    } else {
        fail("int + ptr should be valid");
        for (const auto& error : analyzer.get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 3: Valid Pointer - Integer (ptr - int → pointer)
// ============================================================================
void test_pointer_minus_int() {
    cout << "\n[TEST] Valid: Pointer - Integer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // float* fptr;
    program.push_back(make_unique<VarDecl>("fptr", "float", nullptr, loc, false, nullptr, 1));

    // int n = 2;
    auto n_lit = make_unique<LiteralExpr>("2", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("n", "int", std::move(n_lit), loc));

    // float* prev = fptr - n; (valid)
    auto id_fptr = make_unique<IdentifierExpr>("fptr", loc);
    auto id_n = make_unique<IdentifierExpr>("n", loc);
    auto sub_expr = make_unique<BinaryExpr>(std::move(id_fptr), "-", std::move(id_n), loc);
    program.push_back(make_unique<VarDecl>("prev", "float", std::move(sub_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("ptr - int is valid");
    } else {
        fail("ptr - int should be valid");
        for (const auto& error : analyzer.get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 4: Valid Pointer - Pointer (same type) → int
// ============================================================================
void test_pointer_minus_pointer_same_type() {
    cout << "\n[TEST] Valid: Pointer - Pointer (same type)" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int* start;
    program.push_back(make_unique<VarDecl>("start", "int", nullptr, loc, false, nullptr, 1));

    // int* end;
    program.push_back(make_unique<VarDecl>("end", "int", nullptr, loc, false, nullptr, 1));

    // int diff = end - start; (valid - returns ptrdiff_t/int)
    auto id_end = make_unique<IdentifierExpr>("end", loc);
    auto id_start = make_unique<IdentifierExpr>("start", loc);
    auto sub_expr = make_unique<BinaryExpr>(std::move(id_end), "-", std::move(id_start), loc);
    program.push_back(make_unique<VarDecl>("diff", "int", std::move(sub_expr), loc));

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("ptr - ptr (same type) is valid");
    } else {
        fail("ptr - ptr (same type) should be valid");
        for (const auto& error : analyzer.get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 5: Error - Pointer * Integer
// ============================================================================
void test_pointer_multiply_int_error() {
    cout << "\n[TEST] Error: Pointer * Integer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int* ptr;
    program.push_back(make_unique<VarDecl>("ptr", "int", nullptr, loc, false, nullptr, 1));

    // int scale = 4;
    auto scale_lit = make_unique<LiteralExpr>("4", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("scale", "int", std::move(scale_lit), loc));

    // int* bad = ptr * scale; (INVALID)
    auto id_ptr = make_unique<IdentifierExpr>("ptr", loc);
    auto id_scale = make_unique<IdentifierExpr>("scale", loc);
    auto mul_expr = make_unique<BinaryExpr>(std::move(id_ptr), "*", std::move(id_scale), loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(mul_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "multiply")) {
        pass("ptr * int correctly rejected");
    } else {
        fail("ptr * int should be an error");
    }
}

// ============================================================================
// Test 6: Error - Pointer / Integer
// ============================================================================
void test_pointer_divide_int_error() {
    cout << "\n[TEST] Error: Pointer / Integer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // char* str;
    program.push_back(make_unique<VarDecl>("str", "char", nullptr, loc, false, nullptr, 1));

    // int divisor = 2;
    auto div_lit = make_unique<LiteralExpr>("2", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("divisor", "int", std::move(div_lit), loc));

    // char* bad = str / divisor; (INVALID)
    auto id_str = make_unique<IdentifierExpr>("str", loc);
    auto id_div = make_unique<IdentifierExpr>("divisor", loc);
    auto div_expr = make_unique<BinaryExpr>(std::move(id_str), "/", std::move(id_div), loc);
    program.push_back(make_unique<VarDecl>("bad", "char", std::move(div_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "divide")) {
        pass("ptr / int correctly rejected");
    } else {
        fail("ptr / int should be an error");
    }
}

// ============================================================================
// Test 7: Error - Pointer * Pointer
// ============================================================================
void test_pointer_multiply_pointer_error() {
    cout << "\n[TEST] Error: Pointer * Pointer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int* p1;
    program.push_back(make_unique<VarDecl>("p1", "int", nullptr, loc, false, nullptr, 1));

    // int* p2;
    program.push_back(make_unique<VarDecl>("p2", "int", nullptr, loc, false, nullptr, 1));

    // int* bad = p1 * p2; (INVALID)
    auto id_p1 = make_unique<IdentifierExpr>("p1", loc);
    auto id_p2 = make_unique<IdentifierExpr>("p2", loc);
    auto mul_expr = make_unique<BinaryExpr>(std::move(id_p1), "*", std::move(id_p2), loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(mul_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "multiply")) {
        pass("ptr * ptr correctly rejected");
    } else {
        fail("ptr * ptr should be an error");
    }
}

// ============================================================================
// Test 8: Error - Pointer + Pointer
// ============================================================================
void test_pointer_plus_pointer_error() {
    cout << "\n[TEST] Error: Pointer + Pointer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // float* f1;
    program.push_back(make_unique<VarDecl>("f1", "float", nullptr, loc, false, nullptr, 1));

    // float* f2;
    program.push_back(make_unique<VarDecl>("f2", "float", nullptr, loc, false, nullptr, 1));

    // float* bad = f1 + f2; (INVALID)
    auto id_f1 = make_unique<IdentifierExpr>("f1", loc);
    auto id_f2 = make_unique<IdentifierExpr>("f2", loc);
    auto add_expr = make_unique<BinaryExpr>(std::move(id_f1), "+", std::move(id_f2), loc);
    program.push_back(make_unique<VarDecl>("bad", "float", std::move(add_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "add two pointers")) {
        pass("ptr + ptr correctly rejected");
    } else {
        fail("ptr + ptr should be an error");
    }
}

// ============================================================================
// Test 9: Error - Pointer - Pointer (different types)
// ============================================================================
void test_pointer_minus_pointer_different_types_error() {
    cout << "\n[TEST] Error: Pointer - Pointer (different types)" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int* iptr;
    program.push_back(make_unique<VarDecl>("iptr", "int", nullptr, loc, false, nullptr, 1));

    // char* cptr;
    program.push_back(make_unique<VarDecl>("cptr", "char", nullptr, loc, false, nullptr, 1));

    // int bad = iptr - cptr; (INVALID - different pointer types)
    auto id_iptr = make_unique<IdentifierExpr>("iptr", loc);
    auto id_cptr = make_unique<IdentifierExpr>("cptr", loc);
    auto sub_expr = make_unique<BinaryExpr>(std::move(id_iptr), "-", std::move(id_cptr), loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(sub_expr), loc));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "different types")) {
        pass("ptr - ptr (different types) correctly rejected");
    } else {
        fail("ptr - ptr (different types) should be an error");
    }
}

// ============================================================================
// Test 10: Error - void* Arithmetic
// ============================================================================
void test_void_pointer_arithmetic_error() {
    cout << "\n[TEST] Error: void* Arithmetic" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // void* vptr;
    program.push_back(make_unique<VarDecl>("vptr", "void", nullptr, loc, false, nullptr, 1));

    // int offset = 10;
    auto offset_lit = make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("offset", "int", std::move(offset_lit), loc));

    // void* bad = vptr + offset; (INVALID in standard C)
    auto id_vptr = make_unique<IdentifierExpr>("vptr", loc);
    auto id_offset = make_unique<IdentifierExpr>("offset", loc);
    auto add_expr = make_unique<BinaryExpr>(std::move(id_vptr), "+", std::move(id_offset), loc);
    program.push_back(make_unique<VarDecl>("bad", "void", std::move(add_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "void pointer")) {
        pass("void* + int correctly rejected");
    } else {
        fail("void* + int should be an error (not standard C)");
    }
}

// ============================================================================
// Test 11: Error - Pointer % Integer
// ============================================================================
void test_pointer_modulo_error() {
    cout << "\n[TEST] Error: Pointer % Integer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int* ptr;
    program.push_back(make_unique<VarDecl>("ptr", "int", nullptr, loc, false, nullptr, 1));

    // int mod = 4;
    auto mod_lit = make_unique<LiteralExpr>("4", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("mod", "int", std::move(mod_lit), loc));

    // int* bad = ptr % mod; (INVALID)
    auto id_ptr = make_unique<IdentifierExpr>("ptr", loc);
    auto id_mod = make_unique<IdentifierExpr>("mod", loc);
    auto mod_expr = make_unique<BinaryExpr>(std::move(id_ptr), "%", std::move(id_mod), loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(mod_expr), loc, false, nullptr, 1));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "modulo")) {
        pass("ptr % int correctly rejected");
    } else {
        fail("ptr % int should be an error");
    }
}

// ============================================================================
// Test 12: Type System - Pointer Result Type Validation
// ============================================================================
void test_pointer_result_types() {
    cout << "\n[TEST] Pointer Result Types" << endl;

    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);
    auto int_type = Type::makeInt();

    // Test ptr + int → ptr
    auto result = getArithmeticResultType(*int_ptr, *int_type, "+");
    if (result && result->isPointer() && result->getBaseType() == Type::BaseType::INT) {
        pass("ptr + int returns pointer type");
    } else {
        fail("ptr + int should return pointer type");
    }

    // Test int + ptr → ptr
    result = getArithmeticResultType(*int_type, *int_ptr, "+");
    if (result && result->isPointer() && result->getBaseType() == Type::BaseType::INT) {
        pass("int + ptr returns pointer type");
    } else {
        fail("int + ptr should return pointer type");
    }

    // Test ptr - int → ptr
    result = getArithmeticResultType(*int_ptr, *int_type, "-");
    if (result && result->isPointer() && result->getBaseType() == Type::BaseType::INT) {
        pass("ptr - int returns pointer type");
    } else {
        fail("ptr - int should return pointer type");
    }

    // Test ptr - ptr → int
    auto int_ptr2 = Type::makePointer(Type::BaseType::INT, 1);
    result = getArithmeticResultType(*int_ptr, *int_ptr2, "-");
    if (result && !result->isPointer() && result->getBaseType() == Type::BaseType::INT) {
        pass("ptr - ptr returns int type");
    } else {
        fail("ptr - ptr should return int type");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    cout << "========================================" << endl;
    cout << "User Story #12: Pointer Arithmetic Validation" << endl;
    cout << "========================================" << endl;

    // Valid operations
    test_pointer_plus_int();
    test_int_plus_pointer();
    test_pointer_minus_int();
    test_pointer_minus_pointer_same_type();

    // Invalid operations (should produce errors)
    test_pointer_multiply_int_error();
    test_pointer_divide_int_error();
    test_pointer_multiply_pointer_error();
    test_pointer_plus_pointer_error();
    test_pointer_minus_pointer_different_types_error();
    test_void_pointer_arithmetic_error();
    test_pointer_modulo_error();

    // Type system validation
    test_pointer_result_types();

    cout << "\n========================================" << endl;
    cout << "All pointer arithmetic tests completed!" << endl;
    cout << "========================================\n" << endl;

    return 0;
}
