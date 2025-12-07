/**
 * ==============================================================================
 * Test Helpers and Fixtures for mycc Compiler Test Suite
 * ==============================================================================
 *
 * This header provides common utilities, fixtures, and helper functions used
 * across all test suites. It promotes DRY principles and ensures consistent
 * test patterns throughout the codebase.
 *
 * Key Components:
 *   - Token comparison utilities
 *   - AST comparison utilities
 *   - Test source code generators
 *   - Common assertion macros
 *   - Fixture data structures
 *
 * Usage:
 *   #include "test_helpers.h"
 *
 * ==============================================================================
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include "type.h"

namespace mycc_test {

// ==============================================================================
// Token Testing Utilities
// ==============================================================================

/**
 * Verifies that a token matches the expected type and value.
 *
 * @param token The token to verify
 * @param expected_type The expected TokenType
 * @param expected_value The expected token value (lexeme)
 *
 * Example:
 *   ASSERT_TOKEN_EQ(token, TokenType::KW_INT, "int");
 */
#define ASSERT_TOKEN_EQ(token, expected_type, expected_value) \
    do { \
        ASSERT_EQ(token.type, expected_type) \
            << "Token type mismatch. Got: " << token_type_to_string(token.type) \
            << ", Expected: " << token_type_to_string(expected_type); \
        ASSERT_EQ(std::string(token.value), std::string(expected_value)) \
            << "Token value mismatch"; \
    } while(0)

/**
 * Verifies only the token type (when value doesn't matter).
 */
#define ASSERT_TOKEN_TYPE(token, expected_type) \
    ASSERT_EQ(token.type, expected_type) \
        << "Token type mismatch. Got: " << token_type_to_string(token.type) \
        << ", Expected: " << token_type_to_string(expected_type)

/**
 * Helper to lex a string and return all tokens.
 * Fails the test if lexing produces errors.
 *
 * @param source The source code to lex
 * @return Vector of all tokens (including EOF)
 */
inline std::vector<Token> lex_without_errors(const std::string& source) {
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_FALSE(lexer.hasErrors())
        << "Lexer produced unexpected errors for valid input";

    return tokens;
}

/**
 * Helper to lex a string expecting errors.
 *
 * @param source The source code to lex
 * @param expected_error_count Expected number of errors (0 = any errors)
 * @return Vector of all tokens
 */
inline std::vector<Token> lex_with_errors(
    const std::string& source,
    int expected_error_count = 0
) {
    Lexer lexer(source, "test.c");
    auto tokens = lexer.lexAll();

    EXPECT_TRUE(lexer.hasErrors())
        << "Expected lexer errors but none occurred";

    if (expected_error_count > 0) {
        EXPECT_EQ(lexer.getErrorHandler().get_error_count(), expected_error_count)
            << "Error count mismatch";
    }

    return tokens;
}

// ==============================================================================
// Parser Testing Utilities
// ==============================================================================

/**
 * Helper to parse an expression from source code.
 * Fails the test if parsing produces errors.
 *
 * @param source The source code containing an expression
 * @return Unique pointer to parsed Expression
 */
inline std::unique_ptr<Expression> parse_expression_without_errors(
    const std::string& source
) {
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto expr = parser.parseExpression();

    EXPECT_FALSE(parser.hasErrors())
        << "Parser produced unexpected errors for valid expression";
    EXPECT_NE(expr, nullptr)
        << "Parser returned null expression";

    return expr;
}

/**
 * Helper to parse a complete program.
 *
 * @param source The complete C program source
 * @return Vector of top-level declarations
 */
inline std::vector<std::unique_ptr<Declaration>> parse_program_without_errors(
    const std::string& source
) {
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_FALSE(parser.hasErrors())
        << "Parser produced unexpected errors for valid program";

    return program;
}

/**
 * Helper to parse source expecting errors.
 */
inline std::vector<std::unique_ptr<Declaration>> parse_program_with_errors(
    const std::string& source,
    int expected_error_count = 0
) {
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_TRUE(parser.hasErrors())
        << "Expected parser errors but none occurred";

    if (expected_error_count > 0) {
        EXPECT_EQ(parser.getErrorHandler().get_error_count(), expected_error_count)
            << "Error count mismatch";
    }

    return program;
}

// ==============================================================================
// AST Testing Utilities
// ==============================================================================

/**
 * Type-safe downcast helper for AST nodes.
 * Fails test if cast is invalid.
 *
 * Example:
 *   auto* bin_expr = assert_node_type<BinaryExpression>(expr.get());
 */
template<typename TargetType, typename SourceType>
TargetType* assert_node_type(SourceType* node) {
    EXPECT_NE(node, nullptr) << "Node is null";
    auto* result = dynamic_cast<TargetType*>(node);
    EXPECT_NE(result, nullptr)
        << "AST node type mismatch. Expected: " << typeid(TargetType).name();
    return result;
}

/**
 * Verifies that an identifier expression has the expected name.
 */
inline void assert_identifier(Expression* expr, const std::string& expected_name) {
    auto* ident = assert_node_type<IdentifierExpr>(expr);
    if (ident) {
        ASSERT_EQ(ident->getName(), expected_name)
            << "Identifier name mismatch";
    }
}

/**
 * Verifies that an integer literal has the expected value.
 */
inline void assert_int_literal(Expression* expr, int expected_value) {
    auto* lit = assert_node_type<LiteralExpr>(expr);
    if (lit) {
        ASSERT_EQ(lit->getLiteralType(), LiteralExpr::LiteralType::INTEGER)
            << "Expected INTEGER literal type";
        ASSERT_EQ(std::stoi(lit->getValue()), expected_value)
            << "Integer literal value mismatch";
    }
}

/**
 * Verifies that a binary expression has the expected operator.
 */
inline void assert_binary_op(
    Expression* expr,
    const std::string& expected_op
) {
    auto* bin = assert_node_type<BinaryExpr>(expr);
    if (bin) {
        ASSERT_EQ(bin->getOperator(), expected_op)
            << "Binary operator mismatch";
    }
}

// ==============================================================================
// Sample Source Code Generators
// ==============================================================================

/**
 * Generates a minimal valid C program.
 */
inline std::string minimal_valid_program() {
    return R"(
        int main() {
            return 0;
        }
    )";
}

/**
 * Generates a program with various declarations.
 */
inline std::string sample_declarations_program() {
    return R"(
        int global_var;
        float pi = 3.14;

        int add(int a, int b) {
            return a + b;
        }

        int main() {
            int local = 42;
            return add(local, 10);
        }
    )";
}

/**
 * Generates a program with complex expressions.
 */
inline std::string sample_expressions_program() {
    return R"(
        int main() {
            int a = 5;
            int b = 10;
            int c = (a + b) * 2 - 3;
            int d = a > b ? a : b;
            return c + d;
        }
    )";
}

// ==============================================================================
// Error Testing Utilities
// ==============================================================================

/**
 * Captures error handler output for verification.
 */
class ErrorCapture {
public:
    explicit ErrorCapture(ErrorHandler& handler)
        : handler_(handler)
        , initial_error_count_(handler.get_error_count())
        , initial_warning_count_(handler.get_warning_count())
    {}

    int new_error_count() const {
        return handler_.get_error_count() - initial_error_count_;
    }

    int new_warning_count() const {
        return handler_.get_warning_count() - initial_warning_count_;
    }

    bool has_new_errors() const {
        return new_error_count() > 0;
    }

    bool has_new_warnings() const {
        return new_warning_count() > 0;
    }

private:
    ErrorHandler& handler_;
    int initial_error_count_;
    int initial_warning_count_;
};

// ==============================================================================
// Parameterized Test Data Structures
// ==============================================================================

/**
 * Structure for parameterized tokenization tests.
 */
struct TokenTestCase {
    std::string source;
    TokenType expected_type;
    std::string expected_value;
    std::string description;
};

/**
 * Structure for parameterized operator precedence tests.
 */
struct PrecedenceTestCase {
    std::string expression;
    std::string expected_structure;  // Parenthesized representation
    std::string description;
};

/**
 * Structure for error recovery tests.
 */
struct ErrorTestCase {
    std::string source;
    int expected_error_count;
    std::string error_type;
    std::string description;
};

// ==============================================================================
// Semantic Analysis Testing Utilities
// ==============================================================================

/**
 * Helper to analyze a program and expect no errors.
 *
 * @param source The complete C program source
 * @return SemanticAnalyzer with analysis results
 */
inline SemanticAnalyzer analyze_program_without_errors(const std::string& source) {
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_FALSE(parser.hasErrors())
        << "Parser produced unexpected errors";

    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    EXPECT_FALSE(analyzer.has_errors())
        << "Semantic analyzer produced unexpected errors";

    return analyzer;
}

/**
 * Helper to analyze a program expecting semantic errors.
 *
 * @param source The complete C program source
 * @param expected_error_count Expected number of errors (0 = any errors)
 * @return SemanticAnalyzer with analysis results
 */
inline SemanticAnalyzer analyze_program_with_errors(
    const std::string& source,
    int expected_error_count = 0
) {
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_FALSE(parser.hasErrors())
        << "Parser produced unexpected errors (semantic test expects parse success)";

    SemanticAnalyzer analyzer;
    analyzer.analyze_program(program);

    EXPECT_TRUE(analyzer.has_errors())
        << "Expected semantic errors but none occurred";

    if (expected_error_count > 0) {
        EXPECT_EQ(analyzer.getErrorHandler().get_error_count(), expected_error_count)
            << "Semantic error count mismatch";
    }

    return analyzer;
}

/**
 * Helper to analyze a program expecting warnings (but no errors).
 *
 * @param source The complete C program source
 * @param expected_warning_count Expected number of warnings (0 = any warnings)
 * @return SemanticAnalyzer with analysis results
 */
inline SemanticAnalyzer analyze_program_with_warnings(
    const std::string& source,
    int expected_warning_count = 0
) {
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_FALSE(parser.hasErrors())
        << "Parser produced unexpected errors";

    SemanticAnalyzer analyzer;
    analyzer.set_warnings_enabled(true);
    analyzer.analyze_program(program);

    EXPECT_FALSE(analyzer.has_errors())
        << "Unexpected semantic errors occurred";
    EXPECT_TRUE(analyzer.has_warnings())
        << "Expected semantic warnings but none occurred";

    if (expected_warning_count > 0) {
        EXPECT_EQ(analyzer.getErrorHandler().get_warning_count(), expected_warning_count)
            << "Semantic warning count mismatch";
    }

    return analyzer;
}

/**
 * Structure for parameterized semantic error tests.
 */
struct SemanticErrorTestCase {
    std::string source;
    int expected_error_count;
    std::string error_type;
    std::string description;
};

/**
 * Structure for parameterized type checking tests.
 */
struct TypeCheckTestCase {
    std::string source;
    bool should_pass;
    std::string description;
};

} // namespace mycc_test

#endif // TEST_HELPERS_H
