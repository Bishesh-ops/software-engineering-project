# ==============================================================================
# mycc Compiler - Root CMakeLists.txt
# ==============================================================================
# This is the root CMakeLists.txt for the mycc compiler project.
# It sets up the project and includes the test suite.
#
# The main compiler can still be built using the existing Makefile.
# This CMakeLists.txt is primarily for building and running tests.
#
# Usage:
#   mkdir build && cd build
#   cmake ..
#   make
#   ctest --output-on-failure
# ==============================================================================

cmake_minimum_required(VERSION 3.14)
project(mycc_compiler CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# ==============================================================================
# Compiler Settings
# ==============================================================================

# Add warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==============================================================================
# Include Test Suite
# ==============================================================================

# Add the tests subdirectory
add_subdirectory(tests)

# ==============================================================================
# Project Information
# ==============================================================================

message(STATUS "")
message(STATUS "mycc Compiler - CMake Configuration")
message(STATUS "====================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
message(STATUS "Test Configuration:")
message(STATUS "  - Test framework: GoogleTest (auto-downloaded)")
message(STATUS "  - Test directory: ${CMAKE_SOURCE_DIR}/tests")
message(STATUS "")
message(STATUS "To build tests:")
message(STATUS "  make")
message(STATUS "")
message(STATUS "To run tests:")
message(STATUS "  ctest --output-on-failure")
message(STATUS "  OR")
message(STATUS "  make test")
message(STATUS "====================================")
message(STATUS "")
