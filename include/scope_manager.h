#ifndef SCOPE_MANAGER_H
#define SCOPE_MANAGER_H

#include "symbol_table.h"
#include <vector>
#include <optional>

// ScopeManager manages a stack of symbol tables for nested scopes
// Supports variable shadowing and hierarchical symbol lookup
class ScopeManager {
private:
    // Stack of symbol tables, one per scope level
    // Index 0 is the global scope, higher indices are nested scopes
    std::vector<SymbolTable> scope_stack_;

    // Current scope level (0 = global, 1+ = nested)
    int current_scope_level_;

public:
    // Constructor - automatically creates global scope
    ScopeManager();

    // Enter a new scope (push a new symbol table onto the stack)
    void enter_scope();

    // Exit the current scope (pop the top symbol table)
    // Returns false if attempting to pop global scope, true otherwise
    bool exit_scope();

    // Insert a symbol into the current (innermost) scope
    // Returns true if inserted, false if symbol already exists in current scope
    bool insert(const Symbol& symbol);

    // Lookup a symbol by name, searching from innermost to outermost scope
    // Returns the first matching symbol found, or std::nullopt if not found
    std::optional<Symbol> lookup(const std::string& name) const;

    // Check if a symbol exists in the current scope only
    bool exists_in_current_scope(const std::string& name) const;

    // Check if a symbol exists in any scope (innermost to outermost)
    bool exists_in_any_scope(const std::string& name) const;

    // Get the current scope level (0 = global, 1+ = nested)
    int get_current_scope_level() const { return current_scope_level_; }

    // Get the number of active scopes (always >= 1 because of global scope)
    size_t get_scope_count() const { return scope_stack_.size(); }

    // Check if currently in global scope
    bool is_global_scope() const { return current_scope_level_ == 0; }

    // Clear all scopes and reset to just global scope
    void reset();

    // Get the number of symbols in the current scope
    size_t get_current_scope_size() const;

    // Get the total number of symbols across all scopes
    size_t get_total_symbol_count() const;

    // Get all symbol names from all scopes (for suggestions)
    std::vector<std::string> get_all_symbol_names() const;
};

#endif // SCOPE_MANAGER_H
