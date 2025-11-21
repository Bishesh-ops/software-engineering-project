/**
 * @file test_struct_checking.cpp
 * @brief Test suite for User Story #13: Struct Type Checking
 *
 * Tests:
 * - obj.member - check obj is struct, member exists
 * - ptr->member - check ptr points to struct, member exists
 * - Calculate member offsets
 * - Validate member types
 * - Error detection for invalid member access
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
// Test 1: Basic Struct Member Offset Calculation
// ============================================================================
void test_member_offsets() {
    cout << "\n[TEST] Member Offset Calculation" << endl;

    // Create a simple struct: struct Point { int x; int y; };
    vector<Type::StructMember> members;
    members.emplace_back("x", Type::makeInt());
    members.emplace_back("y", Type::makeInt());

    auto point_type = Type::makeStruct("Point", members);

    // Check offsets
    int x_offset = point_type->getMemberOffset("x");
    int y_offset = point_type->getMemberOffset("y");

    if (x_offset == 0) {
        pass("First member 'x' has offset 0");
    } else {
        fail("First member 'x' should have offset 0, got " + to_string(x_offset));
    }

    if (y_offset == 4) {  // int is 4 bytes
        pass("Second member 'y' has offset 4");
    } else {
        fail("Second member 'y' should have offset 4, got " + to_string(y_offset));
    }

    // Check non-existent member
    if (point_type->getMemberOffset("z") == -1) {
        pass("Non-existent member returns -1");
    } else {
        fail("Non-existent member should return -1");
    }
}

// ============================================================================
// Test 2: Struct Size Calculation
// ============================================================================
void test_struct_size() {
    cout << "\n[TEST] Struct Size Calculation" << endl;

    // struct Point { int x; int y; }; = 4 + 4 = 8 bytes
    vector<Type::StructMember> point_members;
    point_members.emplace_back("x", Type::makeInt());
    point_members.emplace_back("y", Type::makeInt());
    auto point_type = Type::makeStruct("Point", point_members);

    if (point_type->getSizeInBytes() == 8) {
        pass("struct Point size is 8 bytes");
    } else {
        fail("struct Point should be 8 bytes, got " + to_string(point_type->getSizeInBytes()));
    }

    // struct Mixed { char c; int i; double d; }; = 1 + 4 + 8 = 13 bytes (no padding)
    vector<Type::StructMember> mixed_members;
    mixed_members.emplace_back("c", Type::makeChar());
    mixed_members.emplace_back("i", Type::makeInt());
    mixed_members.emplace_back("d", Type::makeDouble());
    auto mixed_type = Type::makeStruct("Mixed", mixed_members);

    if (mixed_type->getSizeInBytes() == 13) {
        pass("struct Mixed size is 13 bytes (simplified, no padding)");
    } else {
        fail("struct Mixed should be 13 bytes, got " + to_string(mixed_type->getSizeInBytes()));
    }
}

// ============================================================================
// Test 3: Member Type Validation
// ============================================================================
void test_member_types() {
    cout << "\n[TEST] Member Type Validation" << endl;

    // struct Data { int id; float value; char* name; };
    vector<Type::StructMember> members;
    members.emplace_back("id", Type::makeInt());
    members.emplace_back("value", Type::makeFloat());
    members.emplace_back("name", Type::makePointer(Type::BaseType::CHAR, 1));

    auto data_type = Type::makeStruct("Data", members);

    // Check member types
    auto id_type = data_type->getMemberType("id");
    if (id_type && id_type->getBaseType() == Type::BaseType::INT) {
        pass("Member 'id' has type int");
    } else {
        fail("Member 'id' should have type int");
    }

    auto value_type = data_type->getMemberType("value");
    if (value_type && value_type->getBaseType() == Type::BaseType::FLOAT) {
        pass("Member 'value' has type float");
    } else {
        fail("Member 'value' should have type float");
    }

    auto name_type = data_type->getMemberType("name");
    if (name_type && name_type->isPointer() && name_type->getBaseType() == Type::BaseType::CHAR) {
        pass("Member 'name' has type char*");
    } else {
        fail("Member 'name' should have type char*");
    }
}

// ============================================================================
// Test 4: Valid Dot Operator (obj.member)
// ============================================================================
void test_valid_dot_operator() {
    cout << "\n[TEST] Valid: Dot Operator (obj.member)" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // struct Point { int x; int y; };
    vector<unique_ptr<VarDecl>> fields;
    fields.push_back(make_unique<VarDecl>("x", "int", nullptr, loc));
    fields.push_back(make_unique<VarDecl>("y", "int", nullptr, loc));
    program.push_back(make_unique<StructDecl>("Point", std::move(fields), loc));

    // struct Point p;
    program.push_back(make_unique<VarDecl>("p", "Point", nullptr, loc));

    // int x_val = p.x; (valid)
    auto id_p = make_unique<IdentifierExpr>("p", loc);
    auto member_access = make_unique<MemberAccessExpr>(std::move(id_p), "x", false, loc);
    program.push_back(make_unique<VarDecl>("x_val", "int", std::move(member_access), loc));

    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("obj.member is valid");
    } else {
        fail("obj.member should be valid");
        for (const auto& error : analyzer.get_errors()) {
            cout << "    Error: " << error.message << endl;
        }
    }
}

// ============================================================================
// Test 5: Error - Dot Operator on Pointer
// ============================================================================
void test_error_dot_on_pointer() {
    cout << "\n[TEST] Error: Dot Operator on Pointer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // struct Point { int x; int y; };
    vector<unique_ptr<VarDecl>> fields;
    fields.push_back(make_unique<VarDecl>("x", "int", nullptr, loc));
    fields.push_back(make_unique<VarDecl>("y", "int", nullptr, loc));
    program.push_back(make_unique<StructDecl>("Point", std::move(fields), loc));

    // struct Point* ptr;
    program.push_back(make_unique<VarDecl>("ptr", "Point", nullptr, loc, false, nullptr, 1));

    // int bad = ptr.x; (INVALID - should use ->)
    auto id_ptr = make_unique<IdentifierExpr>("ptr", loc);
    auto member_access = make_unique<MemberAccessExpr>(std::move(id_ptr), "x", false, loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(member_access), loc));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "did you mean '->'")) {
        pass("Dot on pointer correctly rejected with helpful suggestion");
    } else {
        fail("Dot on pointer should suggest using ->");
    }
}

// ============================================================================
// Test 6: Error - Arrow Operator on Non-Pointer
// ============================================================================
void test_error_arrow_on_non_pointer() {
    cout << "\n[TEST] Error: Arrow Operator on Non-Pointer" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // struct Point { int x; int y; };
    vector<unique_ptr<VarDecl>> fields;
    fields.push_back(make_unique<VarDecl>("x", "int", nullptr, loc));
    fields.push_back(make_unique<VarDecl>("y", "int", nullptr, loc));
    program.push_back(make_unique<StructDecl>("Point", std::move(fields), loc));

    // struct Point p;
    program.push_back(make_unique<VarDecl>("p", "Point", nullptr, loc));

    // int bad = p->x; (INVALID - should use .)
    auto id_p = make_unique<IdentifierExpr>("p", loc);
    auto member_access = make_unique<MemberAccessExpr>(std::move(id_p), "x", true, loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(member_access), loc));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "requires pointer")) {
        pass("Arrow on non-pointer correctly rejected");
    } else {
        fail("Arrow on non-pointer should be rejected");
    }
}

// ============================================================================
// Test 7: Error - Non-Existent Member
// ============================================================================
void test_error_nonexistent_member() {
    cout << "\n[TEST] Error: Non-Existent Member" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // struct Point { int x; int y; };
    vector<unique_ptr<VarDecl>> fields;
    fields.push_back(make_unique<VarDecl>("x", "int", nullptr, loc));
    fields.push_back(make_unique<VarDecl>("y", "int", nullptr, loc));
    program.push_back(make_unique<StructDecl>("Point", std::move(fields), loc));

    // struct Point p;
    program.push_back(make_unique<VarDecl>("p", "Point", nullptr, loc));

    // int bad = p.z; (INVALID - member 'z' doesn't exist)
    auto id_p = make_unique<IdentifierExpr>("p", loc);
    auto member_access = make_unique<MemberAccessExpr>(std::move(id_p), "z", false, loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(member_access), loc));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "no member named")) {
        pass("Non-existent member correctly rejected");
    } else {
        fail("Non-existent member should be rejected");
    }
}

// ============================================================================
// Test 8: Error - Member Access on Non-Struct
// ============================================================================
void test_error_member_access_on_non_struct() {
    cout << "\n[TEST] Error: Member Access on Non-Struct" << endl;

    SourceLocation loc("test.c", 1, 1);
    SemanticAnalyzer analyzer;
    vector<unique_ptr<Declaration>> program;

    // int x = 5;
    auto x_lit = make_unique<LiteralExpr>("5", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("x", "int", std::move(x_lit), loc));

    // int bad = x.member; (INVALID - int is not a struct)
    auto id_x = make_unique<IdentifierExpr>("x", loc);
    auto member_access = make_unique<MemberAccessExpr>(std::move(id_x), "member", false, loc);
    program.push_back(make_unique<VarDecl>("bad", "int", std::move(member_access), loc));

    analyzer.analyze_program(program);

    if (analyzer.has_errors() && contains_error(analyzer.get_errors(), "non-struct")) {
        pass("Member access on non-struct correctly rejected");
    } else {
        fail("Member access on non-struct should be rejected");
    }
}

// ============================================================================
// Test 9: Nested Struct Members
// ============================================================================
void test_nested_struct_offset() {
    cout << "\n[TEST] Nested Struct Member Offsets" << endl;

    // struct Inner { int a; int b; };
    vector<Type::StructMember> inner_members;
    inner_members.emplace_back("a", Type::makeInt());
    inner_members.emplace_back("b", Type::makeInt());
    auto inner_type = Type::makeStruct("Inner", inner_members);

    // struct Outer { int x; struct Inner inner; int y; };
    vector<Type::StructMember> outer_members;
    outer_members.emplace_back("x", Type::makeInt());       // offset 0
    outer_members.emplace_back("inner", inner_type);        // offset 4
    outer_members.emplace_back("y", Type::makeInt());       // offset 4+8=12

    auto outer_type = Type::makeStruct("Outer", outer_members);

    int x_offset = outer_type->getMemberOffset("x");
    int inner_offset = outer_type->getMemberOffset("inner");
    int y_offset = outer_type->getMemberOffset("y");

    if (x_offset == 0 && inner_offset == 4 && y_offset == 12) {
        pass("Nested struct member offsets calculated correctly");
    } else {
        fail("Nested struct offsets incorrect: x=" + to_string(x_offset) +
             ", inner=" + to_string(inner_offset) + ", y=" + to_string(y_offset));
    }
}

// ============================================================================
// Test 10: Type Sizes for Different Types
// ============================================================================
void test_type_sizes() {
    cout << "\n[TEST] Type Size Calculations" << endl;

    bool all_pass = true;

    if (Type::makeChar()->getSizeInBytes() != 1) {
        fail("char should be 1 byte");
        all_pass = false;
    }

    if (Type::makeShort()->getSizeInBytes() != 2) {
        fail("short should be 2 bytes");
        all_pass = false;
    }

    if (Type::makeInt()->getSizeInBytes() != 4) {
        fail("int should be 4 bytes");
        all_pass = false;
    }

    if (Type::makeLong()->getSizeInBytes() != 8) {
        fail("long should be 8 bytes");
        all_pass = false;
    }

    if (Type::makeFloat()->getSizeInBytes() != 4) {
        fail("float should be 4 bytes");
        all_pass = false;
    }

    if (Type::makeDouble()->getSizeInBytes() != 8) {
        fail("double should be 8 bytes");
        all_pass = false;
    }

    if (Type::makePointer(Type::BaseType::INT, 1)->getSizeInBytes() != 8) {
        fail("pointer should be 8 bytes");
        all_pass = false;
    }

    if (all_pass) {
        pass("All basic type sizes are correct");
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================
int main() {
    cout << "========================================" << endl;
    cout << "User Story #13: Struct Type Checking" << endl;
    cout << "========================================" << endl;

    // Offset and size tests
    test_member_offsets();
    test_struct_size();
    test_member_types();
    test_nested_struct_offset();
    test_type_sizes();

    // Valid operations
    test_valid_dot_operator();

    // Error detection
    test_error_dot_on_pointer();
    test_error_arrow_on_non_pointer();
    test_error_nonexistent_member();
    test_error_member_access_on_non_struct();

    cout << "\n========================================" << endl;
    cout << "All struct type checking tests completed!" << endl;
    cout << "========================================\n" << endl;

    return 0;
}
