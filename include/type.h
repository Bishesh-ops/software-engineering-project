#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declaration for struct member types
class Type;

/**
 * @brief Represents types in the C type system
 *
 * Supports:
 * - Base types: int, float, char, void, double, long, short
 * - Pointers: any level of indirection (*, **, ***, etc.)
 * - Arrays: single and multi-dimensional with size information
 * - Structs: named struct types with member information
 * - Type qualifiers: const, volatile (future extension)
 */
class Type {
public:
    /**
     * @brief Base type categories in C
     */
    enum class BaseType {
        VOID,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        STRUCT,
        UNKNOWN
    };

    /**
     * @brief Information about a struct member
     */
    struct StructMember {
        std::string name;
        std::shared_ptr<Type> type;

        StructMember(const std::string& n, std::shared_ptr<Type> t)
            : name(n), type(std::move(t)) {}
    };

private:
    BaseType base_type_;
    int pointer_depth_;
    bool is_array_;
    int array_size_;
    std::string struct_name_;
    std::vector<StructMember> struct_members_;

public:
    // ========================================
    // Constructors
    // ========================================

    /**
     * @brief Construct a simple base type
     * @param base The base type (int, float, etc.)
     */
    explicit Type(BaseType base = BaseType::INT);

    /**
     * @brief Construct a pointer type
     * @param base The base type
     * @param pointer_depth Number of pointer indirections (1 for *, 2 for **, etc.)
     */
    Type(BaseType base, int pointer_depth);

    /**
     * @brief Construct an array type
     * @param base The base type
     * @param array_size Size of the array (0 for unsized arrays)
     */
    Type(BaseType base, int array_size, bool is_array);

    /**
     * @brief Construct a pointer to array type
     * @param base The base type
     * @param pointer_depth Number of pointer indirections
     * @param array_size Size of the array
     */
    Type(BaseType base, int pointer_depth, int array_size);

    /**
     * @brief Construct a struct type
     * @param struct_name Name of the struct
     * @param members List of struct members
     */
    Type(const std::string& struct_name,
         const std::vector<StructMember>& members);

    // ========================================
    // Static Factory Methods (for clarity)
    // ========================================

    static std::shared_ptr<Type> makeInt();
    static std::shared_ptr<Type> makeFloat();
    static std::shared_ptr<Type> makeChar();
    static std::shared_ptr<Type> makeVoid();
    static std::shared_ptr<Type> makeDouble();
    static std::shared_ptr<Type> makeLong();
    static std::shared_ptr<Type> makeShort();

    static std::shared_ptr<Type> makePointer(BaseType base, int depth);
    static std::shared_ptr<Type> makeArray(BaseType base, int size);
    static std::shared_ptr<Type> makeStruct(const std::string& name,
                                           const std::vector<StructMember>& members);

    // Helper to create type from string (for backward compatibility)
    static std::shared_ptr<Type> fromString(const std::string& type_str);

    // ========================================
    // Getters
    // ========================================

    BaseType getBaseType() const { return base_type_; }
    int getPointerDepth() const { return pointer_depth_; }
    bool isArray() const { return is_array_; }
    int getArraySize() const { return array_size_; }
    bool isStruct() const { return base_type_ == BaseType::STRUCT; }
    const std::string& getStructName() const { return struct_name_; }
    const std::vector<StructMember>& getStructMembers() const { return struct_members_; }

    // Type category checks
    bool isPointer() const { return pointer_depth_ > 0; }
    bool isArithmetic() const;
    bool isIntegral() const;
    bool isFloatingPoint() const;
    bool isVoid() const { return base_type_ == BaseType::VOID; }

    // ========================================
    // Type Comparison
    // ========================================

    /**
     * @brief Check if two types are exactly equal
     * @param other The type to compare with
     * @return true if types are identical, false otherwise
     */
    bool equals(const Type& other) const;

    /**
     * @brief Check if two types are compatible (can be assigned)
     * @param other The type to compare with
     * @return true if types are compatible, false otherwise
     *
     * Examples of compatibility:
     * - int and int are compatible
     * - int* and int* are compatible
     * - float and double are compatible (with conversion)
     * - int and float are compatible (with conversion)
     * - void* is compatible with any pointer type
     */
    bool isCompatibleWith(const Type& other) const;

    /**
     * @brief Check if this type can be implicitly converted to another
     * @param target The target type
     * @return true if conversion is possible, false otherwise
     */
    bool canConvertTo(const Type& target) const;

    // ========================================
    // Struct Member Access
    // ========================================

    /**
     * @brief Get the type of a struct member by name
     * @param member_name Name of the member
     * @return Pointer to member type, or nullptr if not found
     */
    std::shared_ptr<Type> getMemberType(const std::string& member_name) const;

    /**
     * @brief Check if struct has a member with given name
     * @param member_name Name of the member
     * @return true if member exists, false otherwise
     */
    bool hasMember(const std::string& member_name) const;

    /**
     * @brief Get the offset of a struct member (USER STORY #13)
     * @param member_name Name of the member
     * @return Byte offset of the member, or -1 if not found
     *
     * Calculates the offset based on member sizes and alignment.
     * Simplified version assumes no padding for now.
     */
    int getMemberOffset(const std::string& member_name) const;

    /**
     * @brief Get the size of this type in bytes (USER STORY #13)
     * @return Size in bytes
     *
     * Returns the size needed for this type.
     * Simplified version for basic types.
     */
    int getSizeInBytes() const;

    // ========================================
    // String Representation
    // ========================================

    /**
     * @brief Convert type to string representation
     * @return String representation (e.g., "int", "float*", "char**", "int[10]")
     */
    std::string toString() const;

    // ========================================
    // Operators
    // ========================================

    bool operator==(const Type& other) const { return equals(other); }
    bool operator!=(const Type& other) const { return !equals(other); }
};

// ========================================
// Helper Functions
// ========================================

/**
 * @brief Get the result type of a binary operation
 * @param left Left operand type
 * @param right Right operand type
 * @param op Operator string (+, -, *, /, etc.)
 * @return Resulting type, or nullptr if operation is invalid
 */
std::shared_ptr<Type> getArithmeticResultType(const Type& left,
                                              const Type& right,
                                              const std::string& op);

/**
 * @brief Check if an operator is valid for given type(s)
 * @param type The operand type
 * @param op Operator string
 * @return true if operator is valid, false otherwise
 */
bool isValidUnaryOperator(const Type& type, const std::string& op);
bool isValidBinaryOperator(const Type& left, const Type& right, const std::string& op);

// ========================================
// USER STORY #11: Implicit Type Conversions
// ========================================

/**
 * @brief Apply integer promotion rules
 * @param type The type to promote
 * @return Promoted type (char/short -> int), or original type if no promotion needed
 *
 * Integer promotion converts char and short to int for use in expressions.
 * This follows C standard integer promotion rules.
 */
std::shared_ptr<Type> applyIntegerPromotion(std::shared_ptr<Type> type);

/**
 * @brief Get the common type for usual arithmetic conversions
 * @param left Left operand type
 * @param right Right operand type
 * @return Common type to which both operands should be converted
 *
 * Implements C's "usual arithmetic conversions" rules:
 * - double > float > long > int > short > char
 * - Both operands are converted to the higher-ranking type
 */
std::shared_ptr<Type> getCommonArithmeticType(std::shared_ptr<Type> left,
                                              std::shared_ptr<Type> right);

/**
 * @brief Apply array-to-pointer decay
 * @param type The array type
 * @return Pointer type if input is array, nullptr otherwise
 *
 * In most contexts, arrays decay to pointers to their first element.
 * For example: int[10] -> int*
 */
std::shared_ptr<Type> applyArrayToPointerDecay(std::shared_ptr<Type> type);

#endif // TYPE_H
