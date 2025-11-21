#include "../../include/symbol_table.h"

// Insert a new symbol into the table
// Returns true if inserted successfully, false if symbol already exists
bool SymbolTable::insert(const Symbol& symbol) {
    // Check if symbol already exists
    if (symbols_.find(symbol.name) != symbols_.end()) {
        return false; // Symbol already exists
    }

    // Insert the symbol
    symbols_[symbol.name] = symbol;
    return true;
}

// Lookup a symbol by name
// Returns the symbol if found, std::nullopt otherwise
std::optional<Symbol> SymbolTable::lookup(const std::string& name) const {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Check if a symbol exists in the table
bool SymbolTable::exists(const std::string& name) const {
    return symbols_.find(name) != symbols_.end();
}

// Remove a symbol from the table
// Returns true if removed, false if symbol didn't exist
bool SymbolTable::remove(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        symbols_.erase(it);
        return true;
    }
    return false;
}

// Get all symbol names in this table
std::vector<std::string> SymbolTable::get_all_names() const {
    std::vector<std::string> names;
    names.reserve(symbols_.size());
    for (const auto& pair : symbols_) {
        names.push_back(pair.first);
    }
    return names;
}
