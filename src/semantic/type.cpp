#include "type.h"
#include <algorithm>
#include <cctype>

// ========================================
// Constructors
// ========================================

Type::Type(BaseType base)
    : base_type_(base),
      pointer_depth_(0),
      is_array_(false),
      array_size_(0),
      struct_name_("") {}

Type::Type(BaseType base, int pointer_depth)
    : base_type_(base),
      pointer_depth_(pointer_depth),
      is_array_(false),
      array_size_(0),
      struct_name_("") {}

Type::Type(BaseType base, int array_size, bool is_array)
    : base_type_(base),
      pointer_depth_(0),
      is_array_(is_array),
      array_size_(array_size),
      struct_name_("") {}

Type::Type(BaseType base, int pointer_depth, int array_size)
    : base_type_(base),
      pointer_depth_(pointer_depth),
      is_array_(true),
      array_size_(array_size),
      struct_name_("") {}

Type::Type(const std::string& struct_name, const std::vector<StructMember>& members)
    : base_type_(BaseType::STRUCT),
      pointer_depth_(0),
      is_array_(false),
      array_size_(0),
      struct_name_(struct_name),
      struct_members_(members) {}

// ========================================
// Static Factory Methods
// ========================================

std::shared_ptr<Type> Type::makeInt() {
    return std::make_shared<Type>(BaseType::INT);
}

std::shared_ptr<Type> Type::makeFloat() {
    return std::make_shared<Type>(BaseType::FLOAT);
}

std::shared_ptr<Type> Type::makeChar() {
    return std::make_shared<Type>(BaseType::CHAR);
}

std::shared_ptr<Type> Type::makeVoid() {
    return std::make_shared<Type>(BaseType::VOID);
}

std::shared_ptr<Type> Type::makeDouble() {
    return std::make_shared<Type>(BaseType::DOUBLE);
}

std::shared_ptr<Type> Type::makeLong() {
    return std::make_shared<Type>(BaseType::LONG);
}

std::shared_ptr<Type> Type::makeShort() {
    return std::make_shared<Type>(BaseType::SHORT);
}

std::shared_ptr<Type> Type::makePointer(BaseType base, int depth) {
    return std::make_shared<Type>(base, depth);
}

std::shared_ptr<Type> Type::makeArray(BaseType base, int size) {
    return std::make_shared<Type>(base, size, true);
}

std::shared_ptr<Type> Type::makeStruct(const std::string& name,
                                      const std::vector<StructMember>& members) {
    return std::make_shared<Type>(name, members);
}

std::shared_ptr<Type> Type::fromString(const std::string& type_str) {
    std::string s = type_str;

    // Count and remove trailing asterisks (pointers)
    int pointer_depth = 0;
    while (!s.empty() && s.back() == '*') {
        pointer_depth++;
        s.pop_back();
    }

    // Trim whitespace
    while (!s.empty() && std::isspace(s.back())) {
        s.pop_back();
    }

    // Parse base type
    BaseType base;
    if (s == "int") base = BaseType::INT;
    else if (s == "float") base = BaseType::FLOAT;
    else if (s == "char") base = BaseType::CHAR;
    else if (s == "void") base = BaseType::VOID;
    else if (s == "double") base = BaseType::DOUBLE;
    else if (s == "long") base = BaseType::LONG;
    else if (s == "short") base = BaseType::SHORT;
    else base = BaseType::UNKNOWN;

    if (pointer_depth > 0) {
        return makePointer(base, pointer_depth);
    }
    return std::make_shared<Type>(base);
}

// ========================================
// Type Category Checks
// ========================================

bool Type::isArithmetic() const {
    // Pointers and arrays are not arithmetic types
    if (pointer_depth_ > 0 || is_array_) {
        return false;
    }
    return isIntegral() || isFloatingPoint();
}

bool Type::isIntegral() const {
    // Pointers and arrays are not integral types
    if (pointer_depth_ > 0 || is_array_) {
        return false;
    }
    return base_type_ == BaseType::CHAR ||
           base_type_ == BaseType::SHORT ||
           base_type_ == BaseType::INT ||
           base_type_ == BaseType::LONG;
}

bool Type::isFloatingPoint() const {
    // Pointers and arrays are not floating point types
    if (pointer_depth_ > 0 || is_array_) {
        return false;
    }
    return base_type_ == BaseType::FLOAT ||
           base_type_ == BaseType::DOUBLE;
}

// ========================================
// Type Comparison
// ========================================

bool Type::equals(const Type& other) const {
    // Base type must match
    if (base_type_ != other.base_type_) {
        return false;
    }

    // Pointer depth must match
    if (pointer_depth_ != other.pointer_depth_) {
        return false;
    }

    // Array status must match
    if (is_array_ != other.is_array_) {
        return false;
    }

    // If array, size must match (0 means unsized, which matches any size)
    if (is_array_) {
        if (array_size_ != 0 && other.array_size_ != 0 &&
            array_size_ != other.array_size_) {
            return false;
        }
    }

    // For structs, names must match
    if (base_type_ == BaseType::STRUCT) {
        return struct_name_ == other.struct_name_;
    }

    return true;
}

bool Type::isCompatibleWith(const Type& other) const {
    // Exact match is always compatible
    if (equals(other)) {
        return true;
    }

    // Pointer types must have same depth
    if (pointer_depth_ != other.pointer_depth_) {
        return false;
    }

    // If both are pointers, check compatibility
    if (pointer_depth_ > 0) {
        // void* is compatible with any pointer type
        if (base_type_ == BaseType::VOID || other.base_type_ == BaseType::VOID) {
            return true;
        }
        // Otherwise base types must match
        return base_type_ == other.base_type_;
    }

    // Array types must match in base type
    if (is_array_ && other.is_array_) {
        return base_type_ == other.base_type_;
    }

    // One is array, other is not - not compatible
    if (is_array_ != other.is_array_) {
        return false;
    }

    // Arithmetic types can be compatible with implicit conversion
    if (isArithmetic() && other.isArithmetic()) {
        return true;
    }

    return false;
}

bool Type::canConvertTo(const Type& target) const {
    // Can always convert to same type
    if (equals(target)) {
        return true;
    }

    // Can't convert void type (non-pointer)
    if ((base_type_ == BaseType::VOID && pointer_depth_ == 0) ||
        (target.base_type_ == BaseType::VOID && target.pointer_depth_ == 0)) {
        return false;
    }

    // Pointer conversions
    if (pointer_depth_ > 0 || target.pointer_depth_ > 0) {
        // Both must be pointers
        if (pointer_depth_ == 0 || target.pointer_depth_ == 0) {
            return false;
        }

        // Pointer depths must match
        if (pointer_depth_ != target.pointer_depth_) {
            return false;
        }

        // Can convert any pointer to void*
        if (target.base_type_ == BaseType::VOID) {
            return true;
        }

        // Can convert void* to any pointer
        if (base_type_ == BaseType::VOID) {
            return true;
        }

        // Otherwise base types must match
        return base_type_ == target.base_type_;
    }

    // Arithmetic type conversions
    if (isArithmetic() && target.isArithmetic()) {
        return true;
    }

    return false;
}

// ========================================
// Struct Member Access
// ========================================

std::shared_ptr<Type> Type::getMemberType(const std::string& member_name) const {
    if (base_type_ != BaseType::STRUCT) {
        return nullptr;
    }

    for (const auto& member : struct_members_) {
        if (member.name == member_name) {
            return member.type;
        }
    }

    return nullptr;
}

bool Type::hasMember(const std::string& member_name) const {
    if (base_type_ != BaseType::STRUCT) {
        return false;
    }

    for (const auto& member : struct_members_) {
        if (member.name == member_name) {
            return true;
        }
    }

    return false;
}

// ========================================
// String Representation
// ========================================

std::string Type::toString() const {
    std::string result;

    // Base type
    switch (base_type_) {
        case BaseType::VOID:   result = "void"; break;
        case BaseType::CHAR:   result = "char"; break;
        case BaseType::SHORT:  result = "short"; break;
        case BaseType::INT:    result = "int"; break;
        case BaseType::LONG:   result = "long"; break;
        case BaseType::FLOAT:  result = "float"; break;
        case BaseType::DOUBLE: result = "double"; break;
        case BaseType::STRUCT: result = "struct " + struct_name_; break;
        case BaseType::UNKNOWN: result = "unknown"; break;
    }

    // Add pointers
    for (int i = 0; i < pointer_depth_; i++) {
        result += "*";
    }

    // Add array notation
    if (is_array_) {
        result += "[";
        if (array_size_ > 0) {
            result += std::to_string(array_size_);
        }
        result += "]";
    }

    return result;
}

// ========================================
// Helper Functions
// ========================================

std::shared_ptr<Type> getArithmeticResultType(const Type& left,
                                              const Type& right,
                                              const std::string& op) {
    // Pointer arithmetic
    if (left.isPointer() || right.isPointer()) {
        // ptr + int or int + ptr -> ptr
        if (op == "+" || op == "-") {
            if (left.isPointer() && right.isIntegral()) {
                return std::make_shared<Type>(left);
            }
            if (left.isIntegral() && right.isPointer() && op == "+") {
                return std::make_shared<Type>(right);
            }
            // ptr - ptr -> int (if same pointer type)
            if (left.isPointer() && right.isPointer() && op == "-") {
                if (left.equals(right)) {
                    return Type::makeInt();
                }
            }
        }
        return nullptr; // Invalid operation
    }

    // Both must be arithmetic types
    if (!left.isArithmetic() || !right.isArithmetic()) {
        return nullptr;
    }

    // Type promotion rules (simplified)
    // double > float > long > int > short > char

    if (left.getBaseType() == Type::BaseType::DOUBLE ||
        right.getBaseType() == Type::BaseType::DOUBLE) {
        return Type::makeDouble();
    }

    if (left.getBaseType() == Type::BaseType::FLOAT ||
        right.getBaseType() == Type::BaseType::FLOAT) {
        return Type::makeFloat();
    }

    if (left.getBaseType() == Type::BaseType::LONG ||
        right.getBaseType() == Type::BaseType::LONG) {
        return Type::makeLong();
    }

    // Default to int for all other integral types
    return Type::makeInt();
}

bool isValidUnaryOperator(const Type& type, const std::string& op) {
    // Pointer dereference
    if (op == "*") {
        return type.isPointer();
    }

    // Address-of
    if (op == "&") {
        return true; // Can take address of any non-void value
    }

    // Arithmetic unary operators
    if (op == "+" || op == "-" || op == "++" || op == "--") {
        return type.isArithmetic() || type.isPointer();
    }

    // Logical NOT
    if (op == "!") {
        return type.isArithmetic() || type.isPointer();
    }

    // Bitwise NOT
    if (op == "~") {
        return type.isIntegral();
    }

    return false;
}

bool isValidBinaryOperator(const Type& left, const Type& right, const std::string& op) {
    // Arithmetic operators
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        // Modulo only for integral types
        if (op == "%") {
            return left.isIntegral() && right.isIntegral();
        }

        // Multiplication and division don't work with pointers
        if (op == "*" || op == "/") {
            return left.isArithmetic() && right.isArithmetic();
        }

        // Check pointer arithmetic for + and -
        if (op == "+" || op == "-") {
            // Pointer + int or int + pointer
            if (left.isPointer() && right.isIntegral()) return true;
            if (left.isIntegral() && right.isPointer() && op == "+") return true;

            // Pointer - pointer (only if same type)
            if (left.isPointer() && right.isPointer() && op == "-") {
                // Only allow subtraction of same pointer types
                return left.equals(right);
            }

            // Regular arithmetic
            if (left.isArithmetic() && right.isArithmetic()) return true;
        }

        return false;
    }

    // Comparison operators
    if (op == "<" || op == ">" || op == "<=" || op == ">=" || op == "==" || op == "!=") {
        // Can compare arithmetic types
        if (left.isArithmetic() && right.isArithmetic()) return true;

        // Can compare pointers of same type (strict equality for different pointer types)
        if (left.isPointer() && right.isPointer()) {
            // For == and !=, allow void* compatibility
            if (op == "==" || op == "!=") {
                return left.isCompatibleWith(right);
            }
            // For <, >, <=, >=, require exact same type
            return left.equals(right);
        }
        return false;
    }

    // Logical operators
    if (op == "&&" || op == "||") {
        // Both must be convertible to boolean (arithmetic or pointer)
        return (left.isArithmetic() || left.isPointer()) &&
               (right.isArithmetic() || right.isPointer());
    }

    // Bitwise operators
    if (op == "&" || op == "|" || op == "^" || op == "<<" || op == ">>") {
        return left.isIntegral() && right.isIntegral();
    }

    return false;
}
