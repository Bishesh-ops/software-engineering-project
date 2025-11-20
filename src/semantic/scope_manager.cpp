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
