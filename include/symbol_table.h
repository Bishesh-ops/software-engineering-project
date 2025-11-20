#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

// Symbol represents an entry in the symbol table
// Stores information about identifiers (variables, functions, etc.)
struct Symbol {
    std::string name;           // Identifier name
    std::string type;           // Type (e.g., "int", "float", "char", etc.)
    int scope_level;            // Scope depth (0 = global, 1+ = nested scopes)
    bool is_function;           // true if this is a function, false for variable
    bool is_array;              // true if this is an array
    int array_size;             // Size of array (0 if not array or size unknown)
    int pointer_depth;          // Number of pointer indirections (0 = not a pointer, 1 = *, 2 = **, etc.)

    // Default constructor (needed for unordered_map)
    Symbol()
        : name(""),
          type(""),
          scope_level(0),
          is_function(false),
          is_array(false),
          array_size(0),
          pointer_depth(0) {}

    // Constructor for variable symbols
    Symbol(const std::string& name,
           const std::string& type,
           int scope_level,
           bool is_array = false,
           int array_size = 0,
           int pointer_depth = 0)
        : name(name),
          type(type),
          scope_level(scope_level),
          is_function(false),
          is_array(is_array),
          array_size(array_size),
          pointer_depth(pointer_depth) {}

    // Tag types for constructor overloading
    struct FunctionTag {};
    static constexpr FunctionTag AsFunction{};

    // Constructor for function symbols (using tag dispatch)
    Symbol(FunctionTag,
           const std::string& name,
           const std::string& return_type,
           int scope_level)
        : name(name),
          type(return_type),
          scope_level(scope_level),
          is_function(true),
          is_array(false),
          array_size(0),
          pointer_depth(0) {}
};

// SymbolTable manages symbol lookup and insertion
// Uses hash map for O(1) average case lookup
class SymbolTable {
private:
    // Hash map: name -> Symbol
    std::unordered_map<std::string, Symbol> symbols_;

public:
    // Constructor
    SymbolTable() = default;

    // Insert a new symbol into the table
    // Returns true if inserted successfully, false if symbol already exists
    bool insert(const Symbol& symbol);

    // Lookup a symbol by name
    // Returns the symbol if found, std::nullopt otherwise
    std::optional<Symbol> lookup(const std::string& name) const;

    // Check if a symbol exists in the table
    bool exists(const std::string& name) const;

    // Get the number of symbols in the table
    size_t size() const { return symbols_.size(); }

    // Clear all symbols from the table
    void clear() { symbols_.clear(); }

    // Remove a symbol from the table (useful for scope management)
    bool remove(const std::string& name);
};

#endif // SYMBOL_TABLE_H
