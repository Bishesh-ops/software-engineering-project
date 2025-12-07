/**
 * ==============================================================================
 * Symbol Table Tests
 * ==============================================================================
 *
 * Module Under Test: SymbolTable (symbol_table.h/cpp)
 *
 * Purpose:
 *   Tests the SymbolTable data structure operations:
 *   - Symbol insertion
 *   - Symbol lookup
 *   - Duplicate detection
 *   - Symbol removal
 *   - Used/unused tracking
 *
 * Coverage:
 *   ✓ Basic insert and lookup operations
 *   ✓ Duplicate symbol detection
 *   ✓ Symbol existence checks
 *   ✓ Symbol removal
 *   ✓ Used/unused variable tracking
 *   ✓ Symbol enumeration
 *
 * ==============================================================================
 */

#include <gtest/gtest.h>
#include "symbol_table.h"
#include "type.h"

class SymbolTableTest : public ::testing::Test {
protected:
    SymbolTable table;

    void SetUp() override {
        // Start with fresh symbol table
        table.clear();
    }
};

// ==============================================================================
// Basic Operations
// ==============================================================================

/**
 * Test: Insert and lookup variable symbol
 *
 * Verifies:
 *   - Can insert symbol
 *   - Can retrieve symbol by name
 */
TEST_F(SymbolTableTest, InsertAndLookupVariable) {
    // Create a simple int variable symbol
    Symbol sym("x", "int", 0);

    // Insert should succeed
    ASSERT_TRUE(table.insert(sym));

    // Lookup should find it
    auto result = table.lookup("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "x");
    EXPECT_EQ(result->type, "int");
}

/**
 * Test: Insert and lookup function symbol
 *
 * Verifies:
 *   - Function symbols can be stored
 *   - is_function flag is preserved
 */
TEST_F(SymbolTableTest, InsertAndLookupFunction) {
    // Create a function symbol
    Symbol func(Symbol::AsFunction, "add", "int", 0);

    ASSERT_TRUE(table.insert(func));

    auto result = table.lookup("add");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->name, "add");
    EXPECT_TRUE(result->is_function);
    EXPECT_EQ(result->type, "int");
}

/**
 * Test: Lookup non-existent symbol
 *
 * Verifies:
 *   - Returns std::nullopt for missing symbol
 */
TEST_F(SymbolTableTest, LookupNonExistent) {
    auto result = table.lookup("nonexistent");
    EXPECT_FALSE(result.has_value());
}

// ==============================================================================
// Duplicate Detection
// ==============================================================================

/**
 * Test: Duplicate symbol insertion fails
 *
 * Verifies:
 *   - Cannot insert two symbols with same name
 */
TEST_F(SymbolTableTest, DuplicateInsertionFails) {
    Symbol sym1("x", "int", 0);
    Symbol sym2("x", "float", 0);  // Different type, same name

    ASSERT_TRUE(table.insert(sym1));
    EXPECT_FALSE(table.insert(sym2))
        << "Second insertion should fail";

    // Verify first symbol is still there
    auto result = table.lookup("x");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->type, "int")
        << "Original symbol should be preserved";
}

/**
 * Test: Symbol exists check
 *
 * Verifies:
 *   - exists() returns true for inserted symbol
 *   - exists() returns false for non-existent symbol
 */
TEST_F(SymbolTableTest, SymbolExistsCheck) {
    Symbol sym("y", "float", 0);
    table.insert(sym);

    EXPECT_TRUE(table.exists("y"));
    EXPECT_FALSE(table.exists("nonexistent"));
}

// ==============================================================================
// Symbol Removal
// ==============================================================================

/**
 * Test: Remove existing symbol
 *
 * Verifies:
 *   - Symbol can be removed
 *   - After removal, lookup fails
 */
TEST_F(SymbolTableTest, RemoveExistingSymbol) {
    Symbol sym("temp", "int", 0);
    table.insert(sym);

    ASSERT_TRUE(table.exists("temp"));

    // Remove should succeed
    EXPECT_TRUE(table.remove("temp"));

    // Symbol should no longer exist
    EXPECT_FALSE(table.exists("temp"));
    EXPECT_FALSE(table.lookup("temp").has_value());
}

/**
 * Test: Remove non-existent symbol
 *
 * Verifies:
 *   - Removing non-existent symbol returns false
 */
TEST_F(SymbolTableTest, RemoveNonExistentSymbol) {
    EXPECT_FALSE(table.remove("nonexistent"));
}

// ==============================================================================
// Symbol Table Size and Clear
// ==============================================================================

/**
 * Test: Symbol table size tracking
 *
 * Verifies:
 *   - Size increases with insertions
 *   - Size decreases with removals
 */
TEST_F(SymbolTableTest, SizeTracking) {
    EXPECT_EQ(table.size(), 0);

    Symbol sym1("a", "int", 0);
    Symbol sym2("b", "float", 0);
    Symbol sym3("c", "char", 0);

    table.insert(sym1);
    EXPECT_EQ(table.size(), 1);

    table.insert(sym2);
    EXPECT_EQ(table.size(), 2);

    table.insert(sym3);
    EXPECT_EQ(table.size(), 3);

    table.remove("b");
    EXPECT_EQ(table.size(), 2);
}

/**
 * Test: Clear symbol table
 *
 * Verifies:
 *   - clear() removes all symbols
 *   - Size becomes 0
 */
TEST_F(SymbolTableTest, ClearTable) {
    Symbol sym1("x", "int", 0);
    Symbol sym2("y", "float", 0);
    table.insert(sym1);
    table.insert(sym2);

    ASSERT_EQ(table.size(), 2);

    table.clear();

    EXPECT_EQ(table.size(), 0);
    EXPECT_FALSE(table.exists("x"));
    EXPECT_FALSE(table.exists("y"));
}

// ==============================================================================
// Used/Unused Tracking
// ==============================================================================

/**
 * Test: Mark symbol as used
 *
 * Verifies:
 *   - Symbols start as unused
 *   - Can be marked as used
 */
TEST_F(SymbolTableTest, MarkSymbolAsUsed) {
    Symbol sym("var", "int", 0);
    ASSERT_FALSE(sym.used);  // Should start unused

    table.insert(sym);
    table.mark_as_used("var");

    auto result = table.lookup("var");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->used);
}

/**
 * Test: Get unused variables
 *
 * Verifies:
 *   - Unused variables are reported
 *   - Used variables are not reported
 */
TEST_F(SymbolTableTest, GetUnusedVariables) {
    Symbol used("used_var", "int", 0);
    Symbol unused1("unused1", "int", 0);
    Symbol unused2("unused2", "float", 0);

    table.insert(used);
    table.insert(unused1);
    table.insert(unused2);

    table.mark_as_used("used_var");

    auto unused = table.get_unused_variables();

    EXPECT_EQ(unused.size(), 2);

    // Check that unused variables are in the list
    bool found_unused1 = false;
    bool found_unused2 = false;

    for (const auto& sym : unused) {
        if (sym.name == "unused1") found_unused1 = true;
        if (sym.name == "unused2") found_unused2 = true;
    }

    EXPECT_TRUE(found_unused1);
    EXPECT_TRUE(found_unused2);
}

/**
 * Test: Functions not reported as unused variables
 *
 * Verifies:
 *   - get_unused_variables() only returns variables, not functions
 */
TEST_F(SymbolTableTest, FunctionsNotInUnusedVariables) {
    Symbol var("unused_var", "int", 0);
    Symbol func(Symbol::AsFunction, "unused_func", "int", 0);

    table.insert(var);
    table.insert(func);

    auto unused = table.get_unused_variables();

    // Should only have the variable, not the function
    EXPECT_EQ(unused.size(), 1);
    EXPECT_EQ(unused[0].name, "unused_var");
}

// ==============================================================================
// Symbol Enumeration
// ==============================================================================

/**
 * Test: Get all symbol names
 *
 * Verifies:
 *   - All inserted symbol names can be retrieved
 */
TEST_F(SymbolTableTest, GetAllSymbolNames) {
    Symbol sym1("alpha", "int", 0);
    Symbol sym2("beta", "float", 0);
    Symbol sym3("gamma", "char", 0);

    table.insert(sym1);
    table.insert(sym2);
    table.insert(sym3);

    auto names = table.get_all_names();

    EXPECT_EQ(names.size(), 3);

    // Check all names are present (order doesn't matter)
    EXPECT_TRUE(std::find(names.begin(), names.end(), "alpha") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "beta") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "gamma") != names.end());
}

// ==============================================================================
// Type System Integration
// ==============================================================================

/**
 * Test: Symbol with new Type system
 *
 * Verifies:
 *   - Can use std::shared_ptr<Type> for symbol types
 */
TEST_F(SymbolTableTest, SymbolWithTypeObject) {
    auto int_type = Type::fromString("int");
    Symbol sym("modern_var", int_type, 0);

    ASSERT_TRUE(table.insert(sym));

    auto result = table.lookup("modern_var");
    ASSERT_TRUE(result.has_value());
    ASSERT_NE(result->symbol_type, nullptr);
    EXPECT_EQ(result->symbol_type->toString(), "int");
}

/**
 * Test: Symbol with pointer type
 *
 * Verifies:
 *   - Pointer types are correctly stored
 */
TEST_F(SymbolTableTest, SymbolWithPointerType) {
    Symbol sym("ptr", "int", 0, false, 0, 1);  // int* (pointer_depth = 1)

    table.insert(sym);

    auto result = table.lookup("ptr");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->pointer_depth, 1);
}

/**
 * Test: Symbol with array type
 *
 * Verifies:
 *   - Array types are correctly stored
 */
TEST_F(SymbolTableTest, SymbolWithArrayType) {
    Symbol sym("arr", "int", 0, true, 10);  // int[10]

    table.insert(sym);

    auto result = table.lookup("arr");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->is_array);
    EXPECT_EQ(result->array_size, 10);
}

// ==============================================================================
// Edge Cases
// ==============================================================================

/**
 * Test: Empty symbol table operations
 *
 * Verifies:
 *   - Operations on empty table don't crash
 */
TEST_F(SymbolTableTest, EmptyTableOperations) {
    EXPECT_EQ(table.size(), 0);
    EXPECT_FALSE(table.exists("anything"));
    EXPECT_FALSE(table.lookup("anything").has_value());
    EXPECT_FALSE(table.remove("anything"));

    auto names = table.get_all_names();
    EXPECT_TRUE(names.empty());

    auto unused = table.get_unused_variables();
    EXPECT_TRUE(unused.empty());
}

/**
 * Test: Symbol names with special characters
 *
 * Verifies:
 *   - Underscores and mixed case work
 */
TEST_F(SymbolTableTest, SpecialCharacterNames) {
    Symbol sym1("_private", "int", 0);
    Symbol sym2("camelCase", "int", 0);
    Symbol sym3("CONSTANT", "int", 0);
    Symbol sym4("__internal__", "int", 0);

    EXPECT_TRUE(table.insert(sym1));
    EXPECT_TRUE(table.insert(sym2));
    EXPECT_TRUE(table.insert(sym3));
    EXPECT_TRUE(table.insert(sym4));

    EXPECT_TRUE(table.exists("_private"));
    EXPECT_TRUE(table.exists("camelCase"));
    EXPECT_TRUE(table.exists("CONSTANT"));
    EXPECT_TRUE(table.exists("__internal__"));
}

/**
 * Test: Large number of symbols
 *
 * Verifies:
 *   - Can handle many symbols efficiently
 */
TEST_F(SymbolTableTest, LargeNumberOfSymbols) {
    const int NUM_SYMBOLS = 1000;

    // Insert many symbols
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        std::string name = "var" + std::to_string(i);
        Symbol sym(name, "int", 0);
        EXPECT_TRUE(table.insert(sym));
    }

    EXPECT_EQ(table.size(), NUM_SYMBOLS);

    // Verify all can be looked up
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        std::string name = "var" + std::to_string(i);
        EXPECT_TRUE(table.exists(name));
    }
}
