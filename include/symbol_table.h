#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include "type.h"
#include "error_handler.h"  // For SourceLocation

// Symbol represents an entry in the symbol table
// Stores information about identifiers (variables, functions, etc.)
struct Symbol {
    std::string name;                    // Identifier name
    std::shared_ptr<Type> symbol_type;   // Type information (new Type class)
    int scope_level;                     // Scope depth (0 = global, 1+ = nested scopes)
    bool is_function;                    // true if this is a function, false for variable
    bool used;                           // Track if variable has been used (for unused warnings)
    SourceLocation declaration_location; // Where this symbol was declared (for warnings)

    // Function-specific fields
    std::vector<std::shared_ptr<Type>> parameter_types;  // Parameter types for functions
    bool is_variadic;                    // true if function accepts variable arguments (...)
    bool is_builtin;                     // true if this is a built-in C library function

    // Deprecated fields (kept for backward compatibility with tests)
    std::string type;           // Type string (deprecated, use symbol_type instead)
    bool is_array;              // true if this is an array (deprecated)
    int array_size;             // Size of array (deprecated)
    int pointer_depth;          // Number of pointer indirections (deprecated)

    // Default constructor (needed for unordered_map)
    Symbol()
        : name(""),
          symbol_type(nullptr),
          scope_level(0),
          is_function(false),
          used(false),
          declaration_location(),
          is_variadic(false),
          is_builtin(false),
          type(""),
          is_array(false),
          array_size(0),
          pointer_depth(0) {}

    // Constructor for variable symbols (backward compatible - uses string type)
    Symbol(const std::string& name,
           const std::string& type,
           int scope_level,
           bool is_array = false,
           int array_size = 0,
           int pointer_depth = 0)
        : name(name),
          symbol_type(createTypeFromLegacyFields(type, is_array, array_size, pointer_depth)),
          scope_level(scope_level),
          is_function(false),
          type(type),
          is_array(is_array),
          array_size(array_size),
          pointer_depth(pointer_depth) {}

    // Constructor for variable symbols (new Type-based)
    Symbol(const std::string& name,
           std::shared_ptr<Type> typ,
           int scope_level)
        : name(name),
          symbol_type(typ),
          scope_level(scope_level),
          is_function(false),
          type(typ ? typ->toString() : ""),
          is_array(typ ? typ->isArray() : false),
          array_size(typ ? typ->getArraySize() : 0),
          pointer_depth(typ ? typ->getPointerDepth() : 0) {}

    // Tag types for constructor overloading
    struct FunctionTag {};
    static constexpr FunctionTag AsFunction{};

    // Constructor for function symbols (using tag dispatch, backward compatible)
    Symbol(FunctionTag,
           const std::string& name,
           const std::string& return_type,
           int scope_level)
        : name(name),
          symbol_type(Type::fromString(return_type)),
          scope_level(scope_level),
          is_function(true),
          type(return_type),
          is_array(false),
          array_size(0),
          pointer_depth(0) {}

    // Constructor for function symbols (new Type-based)
    Symbol(FunctionTag,
           const std::string& name,
           std::shared_ptr<Type> return_type,
           int scope_level)
        : name(name),
          symbol_type(return_type),
          scope_level(scope_level),
          is_function(true),
          type(return_type ? return_type->toString() : ""),
          is_array(false),
          array_size(0),
          pointer_depth(0) {}

    // Constructor for function symbols with parameters
    Symbol(FunctionTag,
           const std::string& name,
           std::shared_ptr<Type> return_type,
           const std::vector<std::shared_ptr<Type>>& params,
           int scope_level)
        : name(name),
          symbol_type(return_type),
          scope_level(scope_level),
          is_function(true),
          parameter_types(params),
          type(return_type ? return_type->toString() : ""),
          is_array(false),
          array_size(0),
          pointer_depth(0) {}

private:
    // Helper to create Type from legacy fields
    static std::shared_ptr<Type> createTypeFromLegacyFields(
        const std::string& type_str,
        bool is_array,
        int array_size,
        int pointer_depth)
    {
        auto base_type = Type::fromString(type_str);
        if (!base_type) {
            return nullptr;
        }

        if (is_array && pointer_depth > 0) {
            // Array of pointers
            return std::make_shared<Type>(
                base_type->getBaseType(),
                pointer_depth,
                array_size
            );
        } else if (is_array) {
            // Just array
            return std::make_shared<Type>(
                base_type->getBaseType(),
                array_size,
                true
            );
        } else if (pointer_depth > 0) {
            // Just pointer
            return std::make_shared<Type>(
                base_type->getBaseType(),
                pointer_depth
            );
        } else {
            // Simple type
            return base_type;
        }
    }
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

    // Get all symbol names in this table
    std::vector<std::string> get_all_names() const;

    // Mark a symbol as used
    void mark_as_used(const std::string& name);

    // Get all unused variable symbols in this table
    std::vector<Symbol> get_unused_variables() const;

    // Get all symbols in this table
    const std::unordered_map<std::string, Symbol>& get_all_symbols() const { return symbols_; }
};

#endif // SYMBOL_TABLE_H
