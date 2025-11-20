/**
 * @file test_semantic.cpp
 * @brief Comprehensive test suite for semantic analysis
 *
 * This program tests all semantic analysis components including:
 * - Type system (base types, pointers, arrays, structs)
 * - Type equality and compatibility checking
 * - Type conversion rules
 * - Symbol table operations (insert, lookup, exists)
 * - Scope stack management (enter/exit scopes, shadowing)
 * - Variable and function symbol handling
 * - SemanticAnalyzer AST walking and declaration registration
 * - Redeclaration error detection
 */

#include "include/type.h"
#include "include/symbol_table.h"
#include "include/scope_manager.h"
#include "include/ast.h"
#include "include/semantic_analyzer.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;

// ============================================================================
// Helper Functions
// ============================================================================

void pass(const string& msg) { cout << "  [PASS] " << msg << endl; }
void fail(const string& msg) { cout << "  [FAIL] " << msg << endl; }

// Helper function to create a function symbol (using tag dispatch)
Symbol createFunctionSymbol(const string& name, const string& return_type, int scope_level) {
    return Symbol(Symbol::AsFunction, name, return_type, scope_level);
}

// ============================================================================
// Type System Tests (User Story #4: Type Representation)
// ============================================================================

void test_type_base_types() {
    cout << "\n[TEST] Type System - Base Types" << endl;

    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto char_type = Type::makeChar();
    auto void_type = Type::makeVoid();

    if (int_type->toString() == "int")
        pass("int type toString()");
    else
        fail("int type toString()");

    if (float_type->toString() == "float")
        pass("float type toString()");
    else
        fail("float type toString()");

    if (char_type->toString() == "char")
        pass("char type toString()");
    else
        fail("char type toString()");

    if (void_type->toString() == "void")
        pass("void type toString()");
    else
        fail("void type toString()");
}

void test_type_pointers() {
    cout << "\n[TEST] Type System - Pointer Types" << endl;

    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);
    auto char_ptr_ptr = Type::makePointer(Type::BaseType::CHAR, 2);
    auto void_ptr = Type::makePointer(Type::BaseType::VOID, 1);

    if (int_ptr->toString() == "int*" && int_ptr->getPointerDepth() == 1)
        pass("int* pointer type");
    else
        fail("int* pointer type");

    if (char_ptr_ptr->toString() == "char**" && char_ptr_ptr->getPointerDepth() == 2)
        pass("char** double pointer");
    else
        fail("char** double pointer");

    if (void_ptr->toString() == "void*")
        pass("void* pointer type");
    else
        fail("void* pointer type");

    if (int_ptr->isPointer() && !Type::makeInt()->isPointer())
        pass("isPointer() correctly identifies pointers");
    else
        fail("isPointer() correctly identifies pointers");
}

void test_type_arrays() {
    cout << "\n[TEST] Type System - Array Types" << endl;

    auto int_array = Type::makeArray(Type::BaseType::INT, 10);
    auto char_array = Type::makeArray(Type::BaseType::CHAR, 256);

    if (int_array->toString() == "int[10]" && int_array->isArray() && int_array->getArraySize() == 10)
        pass("int[10] array type");
    else
        fail("int[10] array type");

    if (char_array->toString() == "char[256]" && char_array->getArraySize() == 256)
        pass("char[256] array type");
    else
        fail("char[256] array type");
}

void test_type_structs() {
    cout << "\n[TEST] Type System - Struct Types" << endl;

    vector<Type::StructMember> members;
    members.emplace_back("x", Type::makeInt());
    members.emplace_back("y", Type::makeInt());
    members.emplace_back("name", Type::makePointer(Type::BaseType::CHAR, 1));

    auto point_struct = Type::makeStruct("Point", members);

    if (point_struct->isStruct() && point_struct->getStructName() == "Point")
        pass("Struct type with name 'Point'");
    else
        fail("Struct type with name 'Point'");

    if (point_struct->hasMember("x") && point_struct->hasMember("y") && point_struct->hasMember("name"))
        pass("Struct has all members (x, y, name)");
    else
        fail("Struct has all members (x, y, name)");

    auto x_type = point_struct->getMemberType("x");
    if (x_type && x_type->equals(*Type::makeInt()))
        pass("Member 'x' has type int");
    else
        fail("Member 'x' has type int");

    auto name_type = point_struct->getMemberType("name");
    if (name_type && name_type->isPointer() && name_type->getPointerDepth() == 1)
        pass("Member 'name' has type char*");
    else
        fail("Member 'name' has type char*");

    if (!point_struct->hasMember("z"))
        pass("Struct does not have non-existent member 'z'");
    else
        fail("Struct does not have non-existent member 'z'");
}

void test_type_equality() {
    cout << "\n[TEST] Type System - Type Equality Checking" << endl;

    auto int1 = Type::makeInt();
    auto int2 = Type::makeInt();
    auto float1 = Type::makeFloat();

    if (int1->equals(*int2))
        pass("int equals int");
    else
        fail("int equals int");

    if (!int1->equals(*float1))
        pass("int does not equal float");
    else
        fail("int does not equal float");

    auto int_ptr1 = Type::makePointer(Type::BaseType::INT, 1);
    auto int_ptr2 = Type::makePointer(Type::BaseType::INT, 1);
    auto int_ptr_ptr = Type::makePointer(Type::BaseType::INT, 2);

    if (int_ptr1->equals(*int_ptr2))
        pass("int* equals int*");
    else
        fail("int* equals int*");

    if (!int_ptr1->equals(*int_ptr_ptr))
        pass("int* does not equal int**");
    else
        fail("int* does not equal int**");

    auto array1 = Type::makeArray(Type::BaseType::INT, 10);
    auto array2 = Type::makeArray(Type::BaseType::INT, 10);
    auto array3 = Type::makeArray(Type::BaseType::INT, 20);

    if (array1->equals(*array2))
        pass("int[10] equals int[10]");
    else
        fail("int[10] equals int[10]");

    if (!array1->equals(*array3))
        pass("int[10] does not equal int[20]");
    else
        fail("int[10] does not equal int[20]");
}

void test_type_compatibility() {
    cout << "\n[TEST] Type System - Type Compatibility" << endl;

    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto char_type = Type::makeChar();

    if (int_type->isCompatibleWith(*float_type))
        pass("int compatible with float (arithmetic types)");
    else
        fail("int compatible with float (arithmetic types)");

    if (float_type->isCompatibleWith(*char_type))
        pass("float compatible with char (arithmetic types)");
    else
        fail("float compatible with char (arithmetic types)");

    auto void_ptr = Type::makePointer(Type::BaseType::VOID, 1);
    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);
    auto char_ptr = Type::makePointer(Type::BaseType::CHAR, 1);

    if (void_ptr->isCompatibleWith(*int_ptr))
        pass("void* compatible with int* (universal pointer)");
    else
        fail("void* compatible with int* (universal pointer)");

    if (int_ptr->isCompatibleWith(*void_ptr))
        pass("int* compatible with void* (universal pointer)");
    else
        fail("int* compatible with void* (universal pointer)");

    if (!int_ptr->isCompatibleWith(*char_ptr))
        pass("int* NOT compatible with char* (different pointer types)");
    else
        fail("int* NOT compatible with char* (different pointer types)");

    auto int_ptr_ptr = Type::makePointer(Type::BaseType::INT, 2);
    if (!int_ptr->isCompatibleWith(*int_ptr_ptr))
        pass("int* NOT compatible with int** (different pointer depth)");
    else
        fail("int* NOT compatible with int** (different pointer depth)");
}

void test_type_conversion() {
    cout << "\n[TEST] Type System - Type Conversion Checking" << endl;

    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto char_type = Type::makeChar();
    auto void_type = Type::makeVoid();

    if (int_type->canConvertTo(*float_type))
        pass("int can convert to float");
    else
        fail("int can convert to float");

    if (char_type->canConvertTo(*int_type))
        pass("char can convert to int");
    else
        fail("char can convert to int");

    if (!int_type->canConvertTo(*void_type))
        pass("int cannot convert to void");
    else
        fail("int cannot convert to void");

    auto void_ptr = Type::makePointer(Type::BaseType::VOID, 1);
    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);

    if (int_ptr->canConvertTo(*void_ptr))
        pass("int* can convert to void*");
    else
        fail("int* can convert to void*");

    if (void_ptr->canConvertTo(*int_ptr))
        pass("void* can convert to int*");
    else
        fail("void* can convert to int*");
}

void test_type_from_string() {
    cout << "\n[TEST] Type System - Creating Types from Strings" << endl;

    auto int_type = Type::fromString("int");
    if (int_type && int_type->toString() == "int")
        pass("Create int from string 'int'");
    else
        fail("Create int from string 'int'");

    auto int_ptr = Type::fromString("int*");
    if (int_ptr && int_ptr->toString() == "int*" && int_ptr->getPointerDepth() == 1)
        pass("Create int* from string 'int*'");
    else
        fail("Create int* from string 'int*'");

    auto char_ptr_ptr = Type::fromString("char**");
    if (char_ptr_ptr && char_ptr_ptr->toString() == "char**" && char_ptr_ptr->getPointerDepth() == 2)
        pass("Create char** from string 'char**'");
    else
        fail("Create char** from string 'char**'");

    auto float_type = Type::fromString("float");
    if (float_type && float_type->toString() == "float")
        pass("Create float from string 'float'");
    else
        fail("Create float from string 'float'");
}

void test_arithmetic_operators() {
    cout << "\n[TEST] Type System - Arithmetic Operator Validation" << endl;

    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);

    if (isValidBinaryOperator(*int_type, *int_type, "+"))
        pass("int + int is valid");
    else
        fail("int + int is valid");

    if (isValidBinaryOperator(*float_type, *int_type, "*"))
        pass("float * int is valid");
    else
        fail("float * int is valid");

    if (isValidBinaryOperator(*int_ptr, *int_type, "+"))
        pass("int* + int is valid (pointer arithmetic)");
    else
        fail("int* + int is valid (pointer arithmetic)");

    if (isValidBinaryOperator(*int_ptr, *int_ptr, "-"))
        pass("int* - int* is valid (pointer difference)");
    else
        fail("int* - int* is valid (pointer difference)");

    if (!isValidBinaryOperator(*int_ptr, *int_ptr, "*"))
        pass("int* * int* is NOT valid (cannot multiply pointers)");
    else
        fail("int* * int* is NOT valid (cannot multiply pointers)");

    if (isValidBinaryOperator(*int_type, *int_type, "%"))
        pass("int % int is valid (modulo)");
    else
        fail("int % int is valid (modulo)");

    if (!isValidBinaryOperator(*float_type, *float_type, "%"))
        pass("float % float is NOT valid (modulo only for integers)");
    else
        fail("float % float is NOT valid (modulo only for integers)");
}

void test_comparison_operators() {
    cout << "\n[TEST] Type System - Comparison Operator Validation" << endl;

    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);
    auto char_ptr = Type::makePointer(Type::BaseType::CHAR, 1);

    if (isValidBinaryOperator(*int_type, *float_type, "<"))
        pass("int < float is valid");
    else
        fail("int < float is valid");

    if (isValidBinaryOperator(*int_ptr, *int_ptr, "=="))
        pass("int* == int* is valid");
    else
        fail("int* == int* is valid");

    if (!isValidBinaryOperator(*int_ptr, *char_ptr, "=="))
        pass("int* == char* is NOT valid (incompatible pointer types)");
    else
        fail("int* == char* is NOT valid (incompatible pointer types)");
}

void test_unary_operators() {
    cout << "\n[TEST] Type System - Unary Operator Validation" << endl;

    auto int_type = Type::makeInt();
    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);
    auto float_type = Type::makeFloat();

    if (isValidUnaryOperator(*int_ptr, "*"))
        pass("*ptr is valid (dereference)");
    else
        fail("*ptr is valid (dereference)");

    if (!isValidUnaryOperator(*int_type, "*"))
        pass("*int is NOT valid (cannot dereference non-pointer)");
    else
        fail("*int is NOT valid (cannot dereference non-pointer)");

    if (isValidUnaryOperator(*int_type, "&"))
        pass("&var is valid (address-of)");
    else
        fail("&var is valid (address-of)");

    if (isValidUnaryOperator(*int_type, "++"))
        pass("++int is valid");
    else
        fail("++int is valid");

    if (isValidUnaryOperator(*float_type, "-"))
        pass("-float is valid");
    else
        fail("-float is valid");

    if (isValidUnaryOperator(*int_type, "~"))
        pass("~int is valid (bitwise NOT)");
    else
        fail("~int is valid (bitwise NOT)");

    if (!isValidUnaryOperator(*float_type, "~"))
        pass("~float is NOT valid (bitwise NOT only for integers)");
    else
        fail("~float is NOT valid (bitwise NOT only for integers)");
}

void test_arithmetic_result_types() {
    cout << "\n[TEST] Type System - Arithmetic Result Type Promotion" << endl;

    auto int_type = Type::makeInt();
    auto float_type = Type::makeFloat();
    auto double_type = Type::makeDouble();
    auto char_type = Type::makeChar();

    auto result1 = getArithmeticResultType(*int_type, *int_type, "+");
    if (result1 && result1->equals(*int_type))
        pass("int + int -> int");
    else
        fail("int + int -> int");

    auto result2 = getArithmeticResultType(*int_type, *float_type, "*");
    if (result2 && result2->equals(*float_type))
        pass("int * float -> float (type promotion)");
    else
        fail("int * float -> float (type promotion)");

    auto result3 = getArithmeticResultType(*float_type, *double_type, "/");
    if (result3 && result3->equals(*double_type))
        pass("float / double -> double (type promotion)");
    else
        fail("float / double -> double (type promotion)");

    auto result4 = getArithmeticResultType(*char_type, *int_type, "+");
    if (result4 && result4->equals(*int_type))
        pass("char + int -> int (char promotes to int)");
    else
        fail("char + int -> int (char promotes to int)");

    auto int_ptr = Type::makePointer(Type::BaseType::INT, 1);
    auto result5 = getArithmeticResultType(*int_ptr, *int_type, "+");
    if (result5 && result5->isPointer())
        pass("int* + int -> int* (pointer arithmetic)");
    else
        fail("int* + int -> int* (pointer arithmetic)");
}

// ============================================================================
// Symbol Table Tests
// ============================================================================

void test_symbol_table_basic() {
    cout << "\n[TEST] Symbol Table - Basic Insert and Lookup" << endl;

    SymbolTable table;

    Symbol var_x("x", "int", 0);
    if (table.insert(var_x))
        pass("Insert variable 'x'");
    else
        fail("Insert variable 'x'");

    auto result = table.lookup("x");
    if (result.has_value() && result->name == "x" && result->type == "int")
        pass("Lookup variable 'x'");
    else
        fail("Lookup variable 'x'");

    auto result2 = table.lookup("y");
    if (!result2.has_value())
        pass("Lookup non-existent variable returns nullopt");
    else
        fail("Lookup non-existent variable returns nullopt");
}

void test_symbol_table_duplicates() {
    cout << "\n[TEST] Symbol Table - Duplicate Prevention" << endl;

    SymbolTable table;
    Symbol var1("count", "int", 0);
    Symbol var2("count", "float", 0);

    if (table.insert(var1))
        pass("First insertion of 'count'");
    else
        fail("First insertion of 'count'");

    if (!table.insert(var2))
        pass("Duplicate insertion prevented");
    else
        fail("Duplicate insertion prevented");

    auto result = table.lookup("count");
    if (result.has_value() && result->type == "int")
        pass("Original symbol preserved");
    else
        fail("Original symbol preserved");
}

void test_symbol_table_arrays() {
    cout << "\n[TEST] Symbol Table - Arrays" << endl;

    SymbolTable table;
    Symbol arr("arr", "int", 0, true, 10, 0);
    table.insert(arr);

    auto result = table.lookup("arr");
    if (result.has_value() && result->is_array && result->array_size == 10 && result->type == "int")
        pass("Array symbol with size 10");
    else
        fail("Array symbol with size 10");
}

void test_symbol_table_pointers() {
    cout << "\n[TEST] Symbol Table - Pointers" << endl;

    SymbolTable table;

    Symbol ptr1("ptr", "int", 0, false, 0, 1);
    table.insert(ptr1);

    auto result1 = table.lookup("ptr");
    if (result1.has_value() && result1->pointer_depth == 1)
        pass("Single pointer (int*)");
    else
        fail("Single pointer (int*)");

    Symbol ptr2("ptr2", "char", 0, false, 0, 2);
    table.insert(ptr2);

    auto result2 = table.lookup("ptr2");
    if (result2.has_value() && result2->pointer_depth == 2)
        pass("Double pointer (char**)");
    else
        fail("Double pointer (char**)");
}

void test_symbol_table_functions() {
    cout << "\n[TEST] Symbol Table - Functions" << endl;

    SymbolTable table;
    Symbol func = createFunctionSymbol("add", "int", 0);
    table.insert(func);

    auto result = table.lookup("add");
    if (result.has_value() && result->is_function && result->type == "int")
        pass("Function symbol with return type");
    else
        fail("Function symbol with return type");
}

// ============================================================================
// Scope Manager Tests
// ============================================================================

void test_scope_manager_initial_state() {
    cout << "\n[TEST] Scope Manager - Initial State" << endl;

    ScopeManager mgr;

    if (mgr.get_current_scope_level() == 0)
        pass("Initial scope level is 0 (global)");
    else
        fail("Initial scope level is 0 (global)");

    if (mgr.get_scope_count() == 1)
        pass("Initial scope count is 1");
    else
        fail("Initial scope count is 1");

    if (mgr.is_global_scope())
        pass("is_global_scope() returns true");
    else
        fail("is_global_scope() returns true");
}

void test_scope_manager_enter_exit() {
    cout << "\n[TEST] Scope Manager - Enter and Exit Scope" << endl;

    ScopeManager mgr;

    mgr.enter_scope();
    if (mgr.get_current_scope_level() == 1 && mgr.get_scope_count() == 2)
        pass("Enter scope: level 1, count 2");
    else
        fail("Enter scope: level 1, count 2");

    mgr.enter_scope();
    if (mgr.get_current_scope_level() == 2 && mgr.get_scope_count() == 3)
        pass("Enter scope: level 2, count 3");
    else
        fail("Enter scope: level 2, count 3");

    bool exit_success = mgr.exit_scope();
    if (exit_success && mgr.get_current_scope_level() == 1 && mgr.get_scope_count() == 2)
        pass("Exit scope: back to level 1");
    else
        fail("Exit scope: back to level 1");

    exit_success = mgr.exit_scope();
    if (exit_success && mgr.get_current_scope_level() == 0 && mgr.is_global_scope())
        pass("Exit scope: back to global");
    else
        fail("Exit scope: back to global");
}

void test_scope_manager_global_protection() {
    cout << "\n[TEST] Scope Manager - Global Scope Protection" << endl;

    ScopeManager mgr;

    bool exit_result = mgr.exit_scope();
    if (!exit_result)
        pass("Cannot exit global scope");
    else
        fail("Cannot exit global scope");

    if (mgr.get_current_scope_level() == 0 && mgr.get_scope_count() == 1)
        pass("Global scope intact after failed exit");
    else
        fail("Global scope intact after failed exit");
}

void test_scope_manager_shadowing() {
    cout << "\n[TEST] Scope Manager - Variable Shadowing" << endl;

    ScopeManager mgr;

    Symbol global_x("x", "int", 0);
    mgr.insert(global_x);

    mgr.enter_scope();
    Symbol local_x("x", "float", 1);
    if (mgr.insert(local_x))
        pass("Can insert shadowing variable 'x'");
    else
        fail("Can insert shadowing variable 'x'");

    auto result = mgr.lookup("x");
    if (result.has_value() && result->type == "float" && result->scope_level == 1)
        pass("Lookup finds shadowing variable (float, level 1)");
    else
        fail("Lookup finds shadowing variable (float, level 1)");

    mgr.exit_scope();

    result = mgr.lookup("x");
    if (result.has_value() && result->type == "int" && result->scope_level == 0)
        pass("After exit, lookup finds original (int, level 0)");
    else
        fail("After exit, lookup finds original (int, level 0)");
}

void test_scope_manager_lookup_order() {
    cout << "\n[TEST] Scope Manager - Lookup Order (Innermost to Outermost)" << endl;

    ScopeManager mgr;

    // Global: a, b
    mgr.insert(Symbol("a", "int", 0));
    mgr.insert(Symbol("b", "int", 0));

    // Level 1: b (shadows), c
    mgr.enter_scope();
    mgr.insert(Symbol("b", "float", 1));
    mgr.insert(Symbol("c", "char", 1));

    // Level 2: c (shadows), d
    mgr.enter_scope();
    mgr.insert(Symbol("c", "double", 2));
    mgr.insert(Symbol("d", "long", 2));

    auto a = mgr.lookup("a");
    auto b = mgr.lookup("b");
    auto c = mgr.lookup("c");
    auto d = mgr.lookup("d");

    bool all_correct =
        a.has_value() && a->type == "int" && a->scope_level == 0 &&
        b.has_value() && b->type == "float" && b->scope_level == 1 &&
        c.has_value() && c->type == "double" && c->scope_level == 2 &&
        d.has_value() && d->type == "long" && d->scope_level == 2;

    if (all_correct)
        pass("Lookup finds correct shadowed variables");
    else
        fail("Lookup finds correct shadowed variables");

    mgr.exit_scope();
    c = mgr.lookup("c");
    d = mgr.lookup("d");

    if (c.has_value() && c->type == "char" && c->scope_level == 1 && !d.has_value())
        pass("After exit, 'c' is level 1, 'd' not found");
    else
        fail("After exit, 'c' is level 1, 'd' not found");
}

void test_scope_manager_exists() {
    cout << "\n[TEST] Scope Manager - Exists Functions" << endl;

    ScopeManager mgr;

    mgr.insert(Symbol("x", "int", 0));

    mgr.enter_scope();
    mgr.insert(Symbol("y", "float", 1));

    bool test1 = !mgr.exists_in_current_scope("x") && mgr.exists_in_any_scope("x");
    bool test2 = mgr.exists_in_current_scope("y") && mgr.exists_in_any_scope("y");
    bool test3 = !mgr.exists_in_current_scope("z") && !mgr.exists_in_any_scope("z");

    if (test1)
        pass("'x' exists in any scope but not current");
    else
        fail("'x' exists in any scope but not current");

    if (test2)
        pass("'y' exists in both current and any scope");
    else
        fail("'y' exists in both current and any scope");

    if (test3)
        pass("'z' exists in neither");
    else
        fail("'z' exists in neither");
}

void test_scope_manager_deep_nesting() {
    cout << "\n[TEST] Scope Manager - Deep Nesting" << endl;

    ScopeManager mgr;

    for (int i = 0; i < 5; i++) {
        mgr.enter_scope();
        Symbol sym("var" + to_string(i), "int", i + 1);
        mgr.insert(sym);
    }

    if (mgr.get_current_scope_level() == 5 && mgr.get_scope_count() == 6)
        pass("5 nested scopes created (level 5, count 6)");
    else
        fail("5 nested scopes created (level 5, count 6)");

    bool all_found = true;
    for (int i = 0; i < 5; i++) {
        auto result = mgr.lookup("var" + to_string(i));
        if (!result.has_value() || result->scope_level != i + 1) {
            all_found = false;
            break;
        }
    }

    if (all_found)
        pass("All 5 variables found with correct scope levels");
    else
        fail("All 5 variables found with correct scope levels");

    for (int i = 0; i < 5; i++) {
        mgr.exit_scope();
    }

    if (mgr.is_global_scope() && mgr.get_scope_count() == 1)
        pass("Exited back to global scope");
    else
        fail("Exited back to global scope");
}

void test_scope_manager_complex_scenario() {
    cout << "\n[TEST] Scope Manager - Complex Realistic Scenario" << endl;

    ScopeManager mgr;

    // Simulate: int x = 10; float y = 3.14;
    mgr.insert(Symbol("x", "int", 0));
    mgr.insert(Symbol("y", "float", 0));

    // Simulate: void foo() { ... }
    mgr.enter_scope();
    mgr.insert(Symbol("a", "int", 1));

    // Simulate: if (a > 0) { ... }
    mgr.enter_scope();
    mgr.insert(Symbol("x", "char", 2));  // Shadow global x
    mgr.insert(Symbol("b", "int", 2));

    auto x = mgr.lookup("x");
    auto y = mgr.lookup("y");
    auto a = mgr.lookup("a");
    auto b = mgr.lookup("b");

    bool in_block_ok =
        x.has_value() && x->type == "char" && x->scope_level == 2 &&
        y.has_value() && y->type == "float" && y->scope_level == 0 &&
        a.has_value() && a->type == "int" && a->scope_level == 1 &&
        b.has_value() && b->type == "int" && b->scope_level == 2;

    if (in_block_ok)
        pass("Inside if block: correct shadowing and lookup");
    else
        fail("Inside if block: correct shadowing and lookup");

    mgr.exit_scope();

    x = mgr.lookup("x");
    b = mgr.lookup("b");

    if (x.has_value() && x->type == "int" && x->scope_level == 0 && !b.has_value())
        pass("After if block: 'x' is global int, 'b' not found");
    else
        fail("After if block: 'x' is global int, 'b' not found");

    mgr.exit_scope();

    a = mgr.lookup("a");
    if (!a.has_value() && mgr.get_total_symbol_count() == 2)
        pass("After function: 'a' not found, 2 global symbols remain");
    else
        fail("After function: 'a' not found, 2 global symbols remain");
}

// ============================================================================
// SemanticAnalyzer Tests - AST Walking and Declaration Registration
// ============================================================================

void test_global_variables() {
    cout << "\n[TEST] Global Variables Registration" << endl;

    // Create AST: int x = 10; float y;
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // int x = 10;
    auto init_x = make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, loc);
    program.push_back(make_unique<VarDecl>("x", "int", move(init_x), loc));

    // float y;
    program.push_back(make_unique<VarDecl>("y", "float", nullptr, loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Global variables registered without errors");
    } else {
        fail("Global variables registered without errors");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_function_registration() {
    cout << "\n[TEST] Functions Registration" << endl;

    // Create AST: int add(int a, int b) { return a + b; }
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // Parameters
    vector<unique_ptr<ParameterDecl>> params;
    params.push_back(make_unique<ParameterDecl>("a", "int", loc));
    params.push_back(make_unique<ParameterDecl>("b", "int", loc));

    // Function body: return a + b;
    auto id_a = make_unique<IdentifierExpr>("a", loc);
    auto id_b = make_unique<IdentifierExpr>("b", loc);
    auto add_expr = make_unique<BinaryExpr>(move(id_a), "+", move(id_b), loc);
    auto return_stmt = make_unique<ReturnStmt>(move(add_expr), loc);

    vector<unique_ptr<Statement>> body_stmts;
    body_stmts.push_back(move(return_stmt));
    auto body = make_unique<CompoundStmt>(move(body_stmts), loc);

    // Function declaration
    program.push_back(make_unique<FunctionDecl>("add", "int", move(params), move(body), loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Function registered without errors");
    } else {
        fail("Function registered without errors");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_parameters_registration() {
    cout << "\n[TEST] Parameters Registration in Function Scope" << endl;

    // Create AST: void foo(int x, float y, char z) { }
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // Parameters
    vector<unique_ptr<ParameterDecl>> params;
    params.push_back(make_unique<ParameterDecl>("x", "int", loc));
    params.push_back(make_unique<ParameterDecl>("y", "float", loc));
    params.push_back(make_unique<ParameterDecl>("z", "char", loc));

    // Empty body
    vector<unique_ptr<Statement>> body_stmts;
    auto body = make_unique<CompoundStmt>(move(body_stmts), loc);

    // Function declaration
    program.push_back(make_unique<FunctionDecl>("foo", "void", move(params), move(body), loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Three parameters registered without errors");
    } else {
        fail("Three parameters registered without errors");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_local_variables_in_blocks() {
    cout << "\n[TEST] Block Scopes with Nested Statements" << endl;

    // Create AST:
    // void foo() {
    //     {
    //         x = 1;
    //     }
    // }
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // Inner block: { x = 1; }
    vector<unique_ptr<Statement>> inner_stmts;
    auto assign = make_unique<AssignmentExpr>(
        make_unique<IdentifierExpr>("x", loc),
        make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, loc),
        loc
    );
    inner_stmts.push_back(make_unique<ExpressionStmt>(move(assign), loc));
    auto inner_block = make_unique<CompoundStmt>(move(inner_stmts), loc);

    // Outer block: { { x = 1; } }
    vector<unique_ptr<Statement>> outer_stmts;
    outer_stmts.push_back(move(inner_block));
    auto outer_block = make_unique<CompoundStmt>(move(outer_stmts), loc);

    // Function
    vector<unique_ptr<ParameterDecl>> params;
    program.push_back(make_unique<FunctionDecl>("foo", "void", move(params), move(outer_block), loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Nested blocks create separate scopes (no errors)");
    } else {
        fail("Nested blocks create separate scopes (no errors)");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_redeclaration_global() {
    cout << "\n[TEST] Redeclaration Errors - Global Scope" << endl;

    // Create AST: int x; int x;  (redeclaration error)
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc1("test.c", 1, 0);
    SourceLocation loc2("test.c", 2, 0);

    program.push_back(make_unique<VarDecl>("x", "int", nullptr, loc1));
    program.push_back(make_unique<VarDecl>("x", "float", nullptr, loc2));  // Redeclaration

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (analyzer.has_errors() && analyzer.get_errors().size() == 1) {
        const auto& err = analyzer.get_errors()[0];
        if (err.message.find("Redeclaration") != string::npos && err.message.find("'x'") != string::npos) {
            pass("Redeclaration error detected for global variable 'x'");
        } else {
            fail("Correct error message for redeclaration");
            cout << "    Got: " << err.message << endl;
        }
    } else {
        fail("Exactly one redeclaration error reported");
        cout << "    Found " << analyzer.get_errors().size() << " errors" << endl;
    }
}

void test_redeclaration_parameters() {
    cout << "\n[TEST] Redeclaration Errors - Function Parameters" << endl;

    // Create AST: void foo(int x, float x) { }  (parameter redeclaration)
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // Parameters with duplicate names
    vector<unique_ptr<ParameterDecl>> params;
    params.push_back(make_unique<ParameterDecl>("x", "int", loc));
    params.push_back(make_unique<ParameterDecl>("x", "float", loc));  // Redeclaration

    // Empty body
    vector<unique_ptr<Statement>> body_stmts;
    auto body = make_unique<CompoundStmt>(move(body_stmts), loc);

    program.push_back(make_unique<FunctionDecl>("foo", "void", move(params), move(body), loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (analyzer.has_errors() && analyzer.get_errors().size() == 1) {
        const auto& err = analyzer.get_errors()[0];
        if (err.message.find("Redeclaration") != string::npos && err.message.find("'x'") != string::npos) {
            pass("Redeclaration error detected for parameter 'x'");
        } else {
            fail("Correct error message for parameter redeclaration");
            cout << "    Got: " << err.message << endl;
        }
    } else {
        fail("Exactly one redeclaration error for parameter");
        cout << "    Found " << analyzer.get_errors().size() << " errors" << endl;
    }
}

void test_redeclaration_local() {
    cout << "\n[TEST] Redeclaration Errors - Parameters and Global" << endl;

    // Create AST:
    // int x;
    // void foo(int x) { }  // OK: different scope (parameter shadows global)
    //
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // Global variable
    program.push_back(make_unique<VarDecl>("x", "int", nullptr, loc));

    // Function with parameter named x (this is legal - parameter shadows global)
    vector<unique_ptr<ParameterDecl>> params;
    params.push_back(make_unique<ParameterDecl>("x", "int", loc));

    vector<unique_ptr<Statement>> body_stmts;
    auto body = make_unique<CompoundStmt>(move(body_stmts), loc);

    program.push_back(make_unique<FunctionDecl>("foo", "void", move(params), move(body), loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Parameter can shadow global variable (no error)");
    } else {
        fail("Parameter shadowing should be legal");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_variable_shadowing() {
    cout << "\n[TEST] Multiple Functions - No Name Conflicts" << endl;

    // Create AST:
    // void foo(int a, int b) { }
    // void bar(int a, int b) { }  // OK: different function scope
    vector<unique_ptr<Declaration>> program;

    SourceLocation loc("test.c", 1, 0);

    // Function foo
    {
        vector<unique_ptr<ParameterDecl>> params;
        params.push_back(make_unique<ParameterDecl>("a", "int", loc));
        params.push_back(make_unique<ParameterDecl>("b", "int", loc));

        vector<unique_ptr<Statement>> body_stmts;
        auto body = make_unique<CompoundStmt>(move(body_stmts), loc);

        program.push_back(make_unique<FunctionDecl>("foo", "void", move(params), move(body), loc));
    }

    // Function bar with same parameter names (legal - different scope)
    {
        vector<unique_ptr<ParameterDecl>> params;
        params.push_back(make_unique<ParameterDecl>("a", "int", loc));
        params.push_back(make_unique<ParameterDecl>("b", "int", loc));

        vector<unique_ptr<Statement>> body_stmts;
        auto body = make_unique<CompoundStmt>(move(body_stmts), loc);

        program.push_back(make_unique<FunctionDecl>("bar", "void", move(params), move(body), loc));
    }

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Different functions can have parameters with same names");
    } else {
        fail("Different functions with same param names should be legal");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_complex_program() {
    cout << "\n[TEST] Complex Program - Multiple Functions and Nested Scopes" << endl;

    // Create AST:
    // int global_x;
    // float global_y;
    //
    // int add(int a, int b) {
    //     return a + b;
    // }
    //
    // void main() {
    //     {
    //         x = 1;
    //     }
    // }

    vector<unique_ptr<Declaration>> program;
    SourceLocation loc("test.c", 1, 0);

    // Globals
    program.push_back(make_unique<VarDecl>("global_x", "int", nullptr, loc));
    program.push_back(make_unique<VarDecl>("global_y", "float", nullptr, loc));

    // Function: add
    {
        vector<unique_ptr<ParameterDecl>> params;
        params.push_back(make_unique<ParameterDecl>("a", "int", loc));
        params.push_back(make_unique<ParameterDecl>("b", "int", loc));

        vector<unique_ptr<Statement>> body_stmts;

        auto id_a = make_unique<IdentifierExpr>("a", loc);
        auto id_b = make_unique<IdentifierExpr>("b", loc);
        auto add_expr = make_unique<BinaryExpr>(move(id_a), "+", move(id_b), loc);
        body_stmts.push_back(make_unique<ReturnStmt>(move(add_expr), loc));

        auto body = make_unique<CompoundStmt>(move(body_stmts), loc);
        program.push_back(make_unique<FunctionDecl>("add", "int", move(params), move(body), loc));
    }

    // Function: main
    {
        vector<unique_ptr<ParameterDecl>> params;

        // Inner block
        vector<unique_ptr<Statement>> inner_stmts;
        auto assign = make_unique<AssignmentExpr>(
            make_unique<IdentifierExpr>("x", loc),
            make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, loc),
            loc
        );
        inner_stmts.push_back(make_unique<ExpressionStmt>(move(assign), loc));
        auto inner_block = make_unique<CompoundStmt>(move(inner_stmts), loc);

        // Outer body
        vector<unique_ptr<Statement>> outer_stmts;
        outer_stmts.push_back(move(inner_block));
        auto body = make_unique<CompoundStmt>(move(outer_stmts), loc);

        program.push_back(make_unique<FunctionDecl>("main", "void", move(params), move(body), loc));
    }

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("Complex program with multiple functions and nested scopes analyzed successfully");
    } else {
        fail("Complex program should have no errors");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

void test_for_loop_scope() {
    cout << "\n[TEST] For Loop Creates Its Own Scope" << endl;

    // Create AST:
    // void foo() {
    //     for (i = 0; i < 10; i++) {
    //         x = 1;
    //     }
    // }

    vector<unique_ptr<Declaration>> program;
    SourceLocation loc("test.c", 1, 0);

    // For loop body: { x = 1; }
    vector<unique_ptr<Statement>> loop_body_stmts;
    auto assign = make_unique<AssignmentExpr>(
        make_unique<IdentifierExpr>("x", loc),
        make_unique<LiteralExpr>("1", LiteralExpr::LiteralType::INTEGER, loc),
        loc
    );
    loop_body_stmts.push_back(make_unique<ExpressionStmt>(move(assign), loc));
    auto loop_body = make_unique<CompoundStmt>(move(loop_body_stmts), loc);

    // For loop: for (i = 0; i < 10; i++)
    auto init_expr = make_unique<AssignmentExpr>(
        make_unique<IdentifierExpr>("i", loc),
        make_unique<LiteralExpr>("0", LiteralExpr::LiteralType::INTEGER, loc),
        loc
    );
    auto init = make_unique<ExpressionStmt>(move(init_expr), loc);
    auto cond = make_unique<BinaryExpr>(
        make_unique<IdentifierExpr>("i", loc),
        "<",
        make_unique<LiteralExpr>("10", LiteralExpr::LiteralType::INTEGER, loc),
        loc
    );
    auto inc = make_unique<UnaryExpr>("++", make_unique<IdentifierExpr>("i", loc), true, loc);
    auto for_stmt = make_unique<ForStmt>(move(init), move(cond), move(inc), move(loop_body), loc);

    // Function body
    vector<unique_ptr<Statement>> func_body_stmts;
    func_body_stmts.push_back(move(for_stmt));
    auto func_body = make_unique<CompoundStmt>(move(func_body_stmts), loc);

    vector<unique_ptr<ParameterDecl>> params;
    program.push_back(make_unique<FunctionDecl>("foo", "void", move(params), move(func_body), loc));

    // Analyze
    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    if (!analyzer.has_errors()) {
        pass("For loop creates separate scope");
    } else {
        fail("For loop should create separate scope");
        for (const auto& err : analyzer.get_errors()) {
            cout << "    Error: " << err.message << endl;
        }
    }
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main() {
    cout << "\n========================================" << endl;
    cout << "COMPREHENSIVE SEMANTIC ANALYSIS TESTS" << endl;
    cout << "========================================" << endl;

    // ========================================
    // PART 1: Type System Tests (User Story #4)
    // ========================================
    cout << "\n--- PART 1: TYPE SYSTEM TESTS (USER STORY #4) ---" << endl;

    test_type_base_types();
    test_type_pointers();
    test_type_arrays();
    test_type_structs();
    test_type_equality();
    test_type_compatibility();
    test_type_conversion();
    test_type_from_string();
    test_arithmetic_operators();
    test_comparison_operators();
    test_unary_operators();
    test_arithmetic_result_types();

    // ========================================
    // PART 2: Symbol Table Tests
    // ========================================
    cout << "\n--- PART 2: SYMBOL TABLE TESTS ---" << endl;

    test_symbol_table_basic();
    test_symbol_table_duplicates();
    test_symbol_table_arrays();
    test_symbol_table_pointers();
    test_symbol_table_functions();

    // ========================================
    // PART 3: Scope Manager Tests
    // ========================================
    cout << "\n--- PART 3: SCOPE MANAGER TESTS ---" << endl;

    test_scope_manager_initial_state();
    test_scope_manager_enter_exit();
    test_scope_manager_global_protection();
    test_scope_manager_shadowing();
    test_scope_manager_lookup_order();
    test_scope_manager_exists();
    test_scope_manager_deep_nesting();
    test_scope_manager_complex_scenario();

    // ========================================
    // PART 4: SemanticAnalyzer - Acceptance Criteria
    // ========================================
    cout << "\n--- PART 4: SEMANTIC ANALYZER (ACCEPTANCE CRITERIA) ---" << endl;

    test_global_variables();              // AC: Adds global variables to global scope
    test_function_registration();         // AC: Adds functions to global scope
    test_parameters_registration();       // AC: Adds parameters to function scope
    test_local_variables_in_blocks();     // AC: Adds local variables to block scopes
    test_redeclaration_global();          // AC: Reports redeclaration errors
    test_redeclaration_parameters();      // AC: Reports redeclaration errors
    test_redeclaration_local();           // AC: Reports redeclaration errors

    // ========================================
    // PART 5: SemanticAnalyzer - Additional Tests
    // ========================================
    cout << "\n--- PART 5: SEMANTIC ANALYZER (ADDITIONAL TESTS) ---" << endl;

    test_variable_shadowing();            // Shadowing across different scopes is legal
    test_complex_program();               // Complex realistic program
    test_for_loop_scope();                // For loops create their own scope

    cout << "\n========================================" << endl;
    cout << "ALL TESTS COMPLETE" << endl;
    cout << "========================================\n" << endl;

    return 0;
}
