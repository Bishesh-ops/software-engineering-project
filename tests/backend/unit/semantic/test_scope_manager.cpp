/**
 * ==============================================================================
 * Scope Manager Tests
 * ==============================================================================
 *
 * Module Under Test: ScopeManager (scope_manager.h/cpp)
 *
 * Purpose:
 *   Tests scope management functionality:
 *   - Scope creation and destruction
 *   - Nested scope handling
 *   - Symbol shadowing
 *   - Hierarchical symbol lookup
 *
 * Coverage:
 *   ✓ Enter and exit scope operations
 *   ✓ Symbol insertion in different scopes
 *   ✓ Symbol lookup across scope hierarchy
 *   ✓ Variable shadowing behavior
 *   ✓ Scope level tracking
 *   ✓ Global scope protection
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "scope_manager.h"
#include "symbol_table.h"

class ScopeManagerTest : public ::testing::Test {
protected:
    ScopeManager scope_mgr;

    void SetUp() override {
        // ScopeManager constructor creates global scope automatically
        ASSERT_EQ(scope_mgr.get_current_scope_level(), 0);
        ASSERT_EQ(scope_mgr.get_scope_count(), 1);
    }
};

// ==============================================================================
// Scope Creation and Destruction
// ==============================================================================

/**
 * Test: Enter new scope
 *
 * Verifies:
 *   - Scope level increments
 *   - Scope count increases
 */
TEST_F(ScopeManagerTest, EnterNewScope) {
    ASSERT_EQ(scope_mgr.get_current_scope_level(), 0);

    scope_mgr.enter_scope();

    EXPECT_EQ(scope_mgr.get_current_scope_level(), 1);
    EXPECT_EQ(scope_mgr.get_scope_count(), 2);
    EXPECT_FALSE(scope_mgr.is_global_scope());
}

/**
 * Test: Exit scope
 *
 * Verifies:
 *   - Can exit non-global scope
 *   - Scope level decrements
 */
TEST_F(ScopeManagerTest, ExitScope) {
    scope_mgr.enter_scope();
    ASSERT_EQ(scope_mgr.get_current_scope_level(), 1);

    bool result = scope_mgr.exit_scope();

    EXPECT_TRUE(result);
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 0);
    EXPECT_EQ(scope_mgr.get_scope_count(), 1);
    EXPECT_TRUE(scope_mgr.is_global_scope());
}

/**
 * Test: Cannot exit global scope
 *
 * Verifies:
 *   - exit_scope() returns false when in global scope
 *   - Global scope is preserved
 */
TEST_F(ScopeManagerTest, CannotExitGlobalScope) {
    ASSERT_TRUE(scope_mgr.is_global_scope());

    bool result = scope_mgr.exit_scope();

    EXPECT_FALSE(result);
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 0);
    EXPECT_TRUE(scope_mgr.is_global_scope());
}

/**
 * Test: Multiple nested scopes
 *
 * Verifies:
 *   - Can create deeply nested scopes
 *   - Scope levels track correctly
 */
TEST_F(ScopeManagerTest, MultipleNestedScopes) {
    scope_mgr.enter_scope();  // Level 1
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 1);

    scope_mgr.enter_scope();  // Level 2
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 2);

    scope_mgr.enter_scope();  // Level 3
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 3);
    EXPECT_EQ(scope_mgr.get_scope_count(), 4);

    // Exit back to global
    scope_mgr.exit_scope();  // Level 2
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 2);

    scope_mgr.exit_scope();  // Level 1
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 1);

    scope_mgr.exit_scope();  // Level 0
    EXPECT_EQ(scope_mgr.get_current_scope_level(), 0);
    EXPECT_TRUE(scope_mgr.is_global_scope());
}

// ==============================================================================
// Symbol Insertion and Lookup
// ==============================================================================

/**
 * Test: Insert symbol in global scope
 *
 * Verifies:
 *   - Can insert global variable
 *   - Can lookup global variable
 */
TEST_F(ScopeManagerTest, InsertGlobalSymbol) {
    Symbol global_var("g_var", "int", 0);

    ASSERT_TRUE(scope_mgr.insert(global_var));
    EXPECT_TRUE(scope_mgr.exists_in_current_scope("g_var"));

    auto result = scope_mgr.lookup("g_var");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "g_var");
    EXPECT_EQ(result->scope_level, 0);
}

/**
 * Test: Insert symbol in nested scope
 *
 * Verifies:
 *   - Can insert local variable
 *   - Local variable has correct scope level
 */
TEST_F(ScopeManagerTest, InsertLocalSymbol) {
    scope_mgr.enter_scope();  // Enter function scope

    Symbol local_var("local", "int", 1);

    ASSERT_TRUE(scope_mgr.insert(local_var));
    EXPECT_TRUE(scope_mgr.exists_in_current_scope("local"));

    auto result = scope_mgr.lookup("local");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "local");
}

/**
 * Test: Lookup from nested scope
 *
 * Verifies:
 *   - Can see global variables from nested scope
 *   - Hierarchical lookup works
 */
TEST_F(ScopeManagerTest, LookupFromNestedScope) {
    Symbol global_var("g_var", "int", 0);
    scope_mgr.insert(global_var);

    scope_mgr.enter_scope();  // Enter nested scope

    // Should be able to see global variable
    auto result = scope_mgr.lookup("g_var");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "g_var");
}

/**
 * Test: Cannot see local variables after exiting scope
 *
 * Verifies:
 *   - Local variables disappear when scope exits
 */
TEST_F(ScopeManagerTest, LocalVariablesDisappearAfterScopeExit) {
    scope_mgr.enter_scope();
    Symbol local_var("local", "int", 1);
    scope_mgr.insert(local_var);

    ASSERT_TRUE(scope_mgr.lookup("local").has_value());

    scope_mgr.exit_scope();

    // Local variable should no longer be visible
    EXPECT_FALSE(scope_mgr.lookup("local").has_value());
}

// ==============================================================================
// Variable Shadowing
// ==============================================================================

/**
 * Test: Local variable shadows global
 *
 * Verifies:
 *   - Local variable with same name hides global
 *   - Lookup finds local, not global
 */
TEST_F(ScopeManagerTest, LocalShadowsGlobal) {
    Symbol global_var("x", "int", 0);
    scope_mgr.insert(global_var);

    scope_mgr.enter_scope();
    Symbol local_var("x", "float", 1);  // Same name, different type
    scope_mgr.insert(local_var);

    // Lookup should find the local variable
    auto result = scope_mgr.lookup("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, "float");
    EXPECT_EQ(result->scope_level, 1);
}

/**
 * Test: Shadowing in deeply nested scopes
 *
 * Verifies:
 *   - Innermost variable shadows all outer ones
 */
TEST_F(ScopeManagerTest, DeepShadowing) {
    Symbol level0("var", "int", 0);
    scope_mgr.insert(level0);

    scope_mgr.enter_scope();  // Level 1
    Symbol level1("var", "float", 1);
    scope_mgr.insert(level1);

    scope_mgr.enter_scope();  // Level 2
    Symbol level2("var", "char", 2);
    scope_mgr.insert(level2);

    // Should find the innermost one
    auto result = scope_mgr.lookup("var");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, "char");
    EXPECT_EQ(result->scope_level, 2);

    // Exit one scope
    scope_mgr.exit_scope();

    // Now should find level 1
    result = scope_mgr.lookup("var");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, "float");
    EXPECT_EQ(result->scope_level, 1);
}

/**
 * Test: No shadowing in same scope
 *
 * Verifies:
 *   - Cannot redeclare variable in same scope
 */
TEST_F(ScopeManagerTest, NoShadowingInSameScope) {
    Symbol var1("x", "int", 0);
    Symbol var2("x", "float", 0);  // Same scope, same name

    ASSERT_TRUE(scope_mgr.insert(var1));
    EXPECT_FALSE(scope_mgr.insert(var2))
        << "Should not allow redeclaration in same scope";

    // First declaration should be preserved
    auto result = scope_mgr.lookup("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, "int");
}

// ==============================================================================
// Scope Query Functions
// ==============================================================================

/**
 * Test: exists_in_current_scope
 *
 * Verifies:
 *   - Only checks current scope, not parent scopes
 */
TEST_F(ScopeManagerTest, ExistsInCurrentScopeOnly) {
    Symbol global_var("g_var", "int", 0);
    scope_mgr.insert(global_var);

    scope_mgr.enter_scope();

    // Global variable should exist in any scope (lookup works)
    EXPECT_TRUE(scope_mgr.exists_in_any_scope("g_var"));

    // But not in current scope specifically
    EXPECT_FALSE(scope_mgr.exists_in_current_scope("g_var"));

    Symbol local_var("local", "int", 1);
    scope_mgr.insert(local_var);

    // Local variable exists in current scope
    EXPECT_TRUE(scope_mgr.exists_in_current_scope("local"));
}

/**
 * Test: exists_in_any_scope
 *
 * Verifies:
 *   - Checks all scopes in hierarchy
 */
TEST_F(ScopeManagerTest, ExistsInAnyScope) {
    Symbol global_var("g_var", "int", 0);
    scope_mgr.insert(global_var);

    scope_mgr.enter_scope();
    scope_mgr.enter_scope();

    // Should find in outer scopes
    EXPECT_TRUE(scope_mgr.exists_in_any_scope("g_var"));
    EXPECT_FALSE(scope_mgr.exists_in_any_scope("nonexistent"));
}

// ==============================================================================
// Reset and State Management
// ==============================================================================

/**
 * Test: Reset scope manager
 *
 * Verifies:
 *   - reset() clears all scopes and returns to global
 */
TEST_F(ScopeManagerTest, ResetScopeManager) {
    // Add some symbols and scopes
    Symbol global_var("g_var", "int", 0);
    scope_mgr.insert(global_var);

    scope_mgr.enter_scope();
    Symbol local_var("local", "int", 1);
    scope_mgr.insert(local_var);

    scope_mgr.enter_scope();

    ASSERT_GT(scope_mgr.get_current_scope_level(), 0);

    // Reset
    scope_mgr.reset();

    EXPECT_EQ(scope_mgr.get_current_scope_level(), 0);
    EXPECT_EQ(scope_mgr.get_scope_count(), 1);
    EXPECT_TRUE(scope_mgr.is_global_scope());

    // All symbols should be gone
    EXPECT_FALSE(scope_mgr.lookup("g_var").has_value());
    EXPECT_FALSE(scope_mgr.lookup("local").has_value());
}

// ==============================================================================
// Symbol Counting
// ==============================================================================

/**
 * Test: Get current scope size
 *
 * Verifies:
 *   - Counts only symbols in current scope
 */
TEST_F(ScopeManagerTest, CurrentScopeSize) {
    Symbol g1("g1", "int", 0);
    Symbol g2("g2", "int", 0);
    scope_mgr.insert(g1);
    scope_mgr.insert(g2);

    EXPECT_EQ(scope_mgr.get_current_scope_size(), 2);

    scope_mgr.enter_scope();

    // New scope starts empty
    EXPECT_EQ(scope_mgr.get_current_scope_size(), 0);

    Symbol l1("l1", "int", 1);
    scope_mgr.insert(l1);

    EXPECT_EQ(scope_mgr.get_current_scope_size(), 1);
}

/**
 * Test: Get total symbol count
 *
 * Verifies:
 *   - Counts symbols across all scopes
 */
TEST_F(ScopeManagerTest, TotalSymbolCount) {
    Symbol g1("g1", "int", 0);
    Symbol g2("g2", "int", 0);
    scope_mgr.insert(g1);
    scope_mgr.insert(g2);

    EXPECT_EQ(scope_mgr.get_total_symbol_count(), 2);

    scope_mgr.enter_scope();
    Symbol l1("l1", "int", 1);
    Symbol l2("l2", "int", 1);
    scope_mgr.insert(l1);
    scope_mgr.insert(l2);

    EXPECT_EQ(scope_mgr.get_total_symbol_count(), 4);
}

// ==============================================================================
// Used/Unused Tracking
// ==============================================================================

/**
 * Test: Mark symbol as used across scopes
 *
 * Verifies:
 *   - Can mark symbols as used
 *   - Searches all scopes to find symbol
 */
TEST_F(ScopeManagerTest, MarkSymbolAsUsed) {
    Symbol var("x", "int", 0);
    scope_mgr.insert(var);

    scope_mgr.enter_scope();

    // Mark from nested scope
    scope_mgr.mark_symbol_as_used("x");

    scope_mgr.exit_scope();

    // Check if marked
    auto result = scope_mgr.lookup("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->used);
}

/**
 * Test: Get unused variables in current scope
 *
 * Verifies:
 *   - Returns only unused symbols from current scope
 */
TEST_F(ScopeManagerTest, GetUnusedVariablesInCurrentScope) {
    Symbol used("used_var", "int", 0);
    Symbol unused("unused_var", "int", 0);

    scope_mgr.insert(used);
    scope_mgr.insert(unused);

    scope_mgr.mark_symbol_as_used("used_var");

    auto unused_vars = scope_mgr.get_unused_variables_in_current_scope();

    EXPECT_EQ(unused_vars.size(), 1);
    EXPECT_EQ(unused_vars[0].name, "unused_var");
}

// ==============================================================================
// Realistic Scenarios
// ==============================================================================

/**
 * Test: Function scope simulation
 *
 * Verifies:
 *   - Global → Function → Block scoping works correctly
 */
TEST_F(ScopeManagerTest, FunctionScopeSimulation) {
    // Global variables
    Symbol global("g_count", "int", 0);
    scope_mgr.insert(global);

    // Enter function scope
    scope_mgr.enter_scope();
    Symbol param("param", "int", 1);
    Symbol local("local", "float", 1);
    scope_mgr.insert(param);
    scope_mgr.insert(local);

    // Enter block scope (e.g., if statement)
    scope_mgr.enter_scope();
    Symbol block_var("i", "int", 2);
    scope_mgr.insert(block_var);

    // All three levels should be accessible
    EXPECT_TRUE(scope_mgr.lookup("g_count").has_value());
    EXPECT_TRUE(scope_mgr.lookup("param").has_value());
    EXPECT_TRUE(scope_mgr.lookup("local").has_value());
    EXPECT_TRUE(scope_mgr.lookup("i").has_value());

    // Exit block
    scope_mgr.exit_scope();

    // Block variable should be gone
    EXPECT_FALSE(scope_mgr.lookup("i").has_value());

    // Function variables still accessible
    EXPECT_TRUE(scope_mgr.lookup("param").has_value());
    EXPECT_TRUE(scope_mgr.lookup("local").has_value());

    // Exit function
    scope_mgr.exit_scope();

    // Only global remains
    EXPECT_TRUE(scope_mgr.lookup("g_count").has_value());
    EXPECT_FALSE(scope_mgr.lookup("param").has_value());
    EXPECT_FALSE(scope_mgr.lookup("local").has_value());
}

/**
 * Test: Multiple functions don't interfere
 *
 * Verifies:
 *   - Exiting one function scope doesn't affect globals
 *   - Can enter new function scope cleanly
 */
TEST_F(ScopeManagerTest, MultipleFunctionsIsolation) {
    // Global
    Symbol global("g_var", "int", 0);
    scope_mgr.insert(global);

    // First function
    scope_mgr.enter_scope();
    Symbol func1_var("x", "int", 1);
    scope_mgr.insert(func1_var);
    ASSERT_TRUE(scope_mgr.lookup("x").has_value());
    scope_mgr.exit_scope();

    // Second function
    scope_mgr.enter_scope();
    Symbol func2_var("x", "float", 1);  // Same name, different function
    scope_mgr.insert(func2_var);

    auto result = scope_mgr.lookup("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, "float");  // Should be the new one

    scope_mgr.exit_scope();

    // Global still accessible
    EXPECT_TRUE(scope_mgr.lookup("g_var").has_value());
}
