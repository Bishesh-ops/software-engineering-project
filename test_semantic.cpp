/**
 * @file test_semantic.cpp
 * @brief Test suite for semantic analysis components (Symbol Table and Scope Manager)
 *
 * This program tests the semantic analysis infrastructure including:
 * - Symbol table operations (insert, lookup, exists)
 * - Scope stack management (enter/exit scopes, shadowing)
 * - Variable and function symbol handling
 */

#include "include/symbol_table.h"
#include "include/scope_manager.h"
#include <iostream>
#include <string>

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
// Main Test Runner
// ============================================================================

int main() {
    cout << "\n========================================" << endl;
    cout << "SEMANTIC ANALYSIS TESTS" << endl;
    cout << "========================================" << endl;

    // Symbol Table Tests
    test_symbol_table_basic();
    test_symbol_table_duplicates();
    test_symbol_table_arrays();
    test_symbol_table_pointers();
    test_symbol_table_functions();

    // Scope Manager Tests
    test_scope_manager_initial_state();
    test_scope_manager_enter_exit();
    test_scope_manager_global_protection();
    test_scope_manager_shadowing();
    test_scope_manager_lookup_order();
    test_scope_manager_exists();
    test_scope_manager_deep_nesting();
    test_scope_manager_complex_scenario();

    cout << "\n========================================" << endl;
    cout << "TESTS COMPLETE" << endl;
    cout << "========================================\n" << endl;

    return 0;
}
