# ==============================================================================
# mycc Compiler - Testing Infrastructure
# ==============================================================================
# This CMakeLists.txt configures the complete test suite for the mycc compiler.
# It uses GoogleTest as the testing framework and provides comprehensive
# coverage across all compiler modules.
#
# Test Organization:
#   - unit/         : Isolated component tests
#   - integration/  : Multi-component pipeline tests
#   - fixtures/     : Shared test data and helpers
#
# Usage:
#   mkdir build && cd build
#   cmake ..
#   make
#   ctest --output-on-failure
# ==============================================================================

cmake_minimum_required(VERSION 3.14)

# ==============================================================================
# GoogleTest Integration
# ==============================================================================
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing functionality
enable_testing()
include(GoogleTest)

# ==============================================================================
# Test Configuration
# ==============================================================================

# Include directories for test code
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/backend/fixtures
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags for tests (enable warnings, sanitizers in debug mode)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(-Wall -Wextra -Wpedantic -g)
        # Optional: Enable AddressSanitizer for memory error detection
        # add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        # add_link_options(-fsanitize=address)
    endif()
endif()

# ==============================================================================
# Test Utilities and Fixtures
# ==============================================================================

# Helper library for test fixtures and common utilities
add_library(test_helpers INTERFACE)
target_include_directories(test_helpers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/fixtures
)

# ==============================================================================
# Backend Unit Tests - Lexer
# ==============================================================================

add_executable(test_lexer_basic
    backend/unit/lexer/test_lexer_basic.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_lexer_basic
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_lexer_basic)

add_executable(test_lexer_keywords
    backend/unit/lexer/test_lexer_keywords.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_lexer_keywords
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_lexer_keywords)

add_executable(test_lexer_operators
    backend/unit/lexer/test_lexer_operators.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_lexer_operators
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_lexer_operators)

add_executable(test_lexer_literals
    backend/unit/lexer/test_lexer_literals.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_lexer_literals
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_lexer_literals)

add_executable(test_lexer_error_recovery
    backend/unit/lexer/test_lexer_error_recovery.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_lexer_error_recovery
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_lexer_error_recovery)

# ==============================================================================
# Backend Unit Tests - Parser
# ==============================================================================

add_executable(test_parser_expressions
    backend/unit/parser/test_parser_expressions.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/AST/ast_printer.cpp
)
target_link_libraries(test_parser_expressions
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_parser_expressions)

add_executable(test_parser_statements
    backend/unit/parser/test_parser_statements.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/AST/ast_printer.cpp
)
target_link_libraries(test_parser_statements
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_parser_statements)

add_executable(test_parser_declarations
    backend/unit/parser/test_parser_declarations.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/AST/ast_printer.cpp
)
target_link_libraries(test_parser_declarations
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_parser_declarations)

# ==============================================================================
# Backend Unit Tests - Semantic Analyzer
# ==============================================================================

add_executable(test_type_checking
    backend/unit/semantic/test_type_checking.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/semantic_analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/scope_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/type.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/AST/ast_printer.cpp
)
target_link_libraries(test_type_checking
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_type_checking)

add_executable(test_symbol_table
    backend/unit/semantic/test_symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/type.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_symbol_table
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_symbol_table)

add_executable(test_scope_manager
    backend/unit/semantic/test_scope_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/scope_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/type.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
)
target_link_libraries(test_scope_manager
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_scope_manager)

add_executable(test_semantic_errors
    backend/unit/semantic/test_semantic_errors.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/semantic_analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/scope_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/type.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/AST/ast_printer.cpp
)
target_link_libraries(test_semantic_errors
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_semantic_errors)

add_executable(test_warnings
    backend/unit/semantic/test_warnings.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/semantic_analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/symbol_table.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/scope_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/semantic/type.cpp
    ${CMAKE_SOURCE_DIR}/src/parser/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/lexer.cpp
    ${CMAKE_SOURCE_DIR}/src/lexer/token.cpp
    ${CMAKE_SOURCE_DIR}/src/error/error_handler.cpp
    ${CMAKE_SOURCE_DIR}/src/AST/ast_printer.cpp
)
target_link_libraries(test_warnings
    GTest::gtest_main
    test_helpers
)
gtest_discover_tests(test_warnings)

# ==============================================================================
# Test Summary Target
# ==============================================================================

# Custom target to run all tests with verbose output
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS
        test_lexer_basic
        test_lexer_keywords
        test_lexer_operators
        test_lexer_literals
        test_lexer_error_recovery
        test_parser_expressions
        test_parser_statements
        test_parser_declarations
        test_type_checking
        test_symbol_table
        test_scope_manager
        test_semantic_errors
        test_warnings
    COMMENT "Running all mycc compiler tests..."
)

# ==============================================================================
# Code Coverage (Optional)
# ==============================================================================
# Uncomment to enable code coverage reporting (requires lcov/gcov)
# if(CMAKE_BUILD_TYPE MATCHES Debug)
#     if(CMAKE_COMPILER_IS_GNUCXX)
#         include(CodeCoverage)
#         setup_target_for_coverage(coverage ctest coverage)
#     endif()
# endif()
