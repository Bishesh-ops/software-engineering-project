#include "../../include/scope_manager.h"

// Constructor - automatically creates global scope
ScopeManager::ScopeManager() : current_scope_level_(0) {
    // Initialize with global scope (scope level 0)
    scope_stack_.push_back(SymbolTable());
}

// Enter a new scope (push a new symbol table onto the stack)
void ScopeManager::enter_scope() {
    current_scope_level_++;
    scope_stack_.push_back(SymbolTable());
}

// Exit the current scope (pop the top symbol table)
// Returns false if attempting to pop global scope, true otherwise
bool ScopeManager::exit_scope() {
    // Protect global scope - cannot pop it
    if (current_scope_level_ == 0 || scope_stack_.size() <= 1) {
        return false;
    }

    scope_stack_.pop_back();
    current_scope_level_--;
    return true;
}

// Insert a symbol into the current (innermost) scope
// Returns true if inserted, false if symbol already exists in current scope
bool ScopeManager::insert(const Symbol& symbol) {
    if (scope_stack_.empty()) {
        return false;
    }

    // Create a new symbol with the current scope level
    Symbol scoped_symbol = symbol;
    scoped_symbol.scope_level = current_scope_level_;

    // Insert into the current (top) scope
    return scope_stack_.back().insert(scoped_symbol);
}

// Lookup a symbol by name, searching from innermost to outermost scope
// Returns the first matching symbol found, or std::nullopt if not found
std::optional<Symbol> ScopeManager::lookup(const std::string& name) const {
    // Search from innermost (top of stack) to outermost (bottom of stack)
    for (auto it = scope_stack_.rbegin(); it != scope_stack_.rend(); ++it) {
        auto result = it->lookup(name);
        if (result.has_value()) {
            return result;
        }
    }

    // Not found in any scope
    return std::nullopt;
}

// Check if a symbol exists in the current scope only
bool ScopeManager::exists_in_current_scope(const std::string& name) const {
    if (scope_stack_.empty()) {
        return false;
    }
    return scope_stack_.back().exists(name);
}

// Check if a symbol exists in any scope (innermost to outermost)
bool ScopeManager::exists_in_any_scope(const std::string& name) const {
    return lookup(name).has_value();
}

// Clear all scopes and reset to just global scope
void ScopeManager::reset() {
    scope_stack_.clear();
    current_scope_level_ = 0;
    scope_stack_.push_back(SymbolTable());
}

// Get the number of symbols in the current scope
size_t ScopeManager::get_current_scope_size() const {
    if (scope_stack_.empty()) {
        return 0;
    }
    return scope_stack_.back().size();
}

// Get the total number of symbols across all scopes
size_t ScopeManager::get_total_symbol_count() const {
    size_t total = 0;
    for (const auto& table : scope_stack_) {
        total += table.size();
    }
    return total;
}

// Get all symbol names from all scopes
std::vector<std::string> ScopeManager::get_all_symbol_names() const {
    std::vector<std::string> all_names;

    // Collect names from all scopes (innermost to outermost)
    for (const auto& table : scope_stack_) {
        auto names = table.get_all_names();
        all_names.insert(all_names.end(), names.begin(), names.end());
    }

    return all_names;
}

// Mark a symbol as used (searches all scopes from innermost to outermost)
void ScopeManager::mark_symbol_as_used(const std::string& name) {
    // Search from innermost (top of stack) to outermost (bottom of stack)
    for (auto it = scope_stack_.rbegin(); it != scope_stack_.rend(); ++it) {
        if (it->exists(name)) {
            it->mark_as_used(name);
            return;  // Found and marked, done
        }
    }
}

// Get all unused variables from the current scope
std::vector<Symbol> ScopeManager::get_unused_variables_in_current_scope() const {
    if (scope_stack_.empty()) {
        return std::vector<Symbol>();
    }
    return scope_stack_.back().get_unused_variables();
}
