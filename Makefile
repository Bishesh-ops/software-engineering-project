# --- Compiler and Flags ---
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g
# macOS uses LLVM linker, Linux uses GNU ld
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    LDFLAGS :=
else
    LDFLAGS := -Wl,--allow-multiple-definition
endif
INCLUDES := -Iinclude

# --- Directories ---
OBJ_DIR := build/obj
BIN_DIR := bin

# --- Source Files ---
LEXER_SRCS := src/lexer/lexer.cpp src/lexer/token.cpp
PARSER_SRCS := src/parser/parser.cpp
AST_SRCS := src/AST/ast_printer.cpp
SEMANTIC_SRCS := src/semantic/type.cpp src/semantic/symbol_table.cpp src/semantic/scope_manager.cpp src/semantic/semantic_analyzer.cpp
TEST_LEXER_SRCS := tests/test_lexer.cpp
TEST_PARSER_SRCS := tests/test_parser.cpp
TEST_SEMANTIC_MAIN_SRCS := tests/test_semantic_main.cpp
TEST_SEMANTIC_US11_SRCS := tests/test_semantic_us11_implicit_conversions.cpp
TEST_SEMANTIC_US12_SRCS := tests/test_semantic_us12_pointer_arithmetic.cpp
TEST_SEMANTIC_US13_SRCS := tests/test_semantic_us13_struct_checking.cpp
TEST_INTEGRATION_SRCS := tests/test_integration.cpp

# --- Object Files ---
LEXER_OBJS := $(LEXER_SRCS:src/lexer/%.cpp=$(OBJ_DIR)/%.o)
PARSER_OBJS := $(PARSER_SRCS:src/parser/%.cpp=$(OBJ_DIR)/%.o)
AST_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(AST_SRCS))
SEMANTIC_OBJS := $(SEMANTIC_SRCS:src/semantic/%.cpp=$(OBJ_DIR)/%.o)
TEST_LEXER_OBJS := $(TEST_LEXER_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_PARSER_OBJS := $(TEST_PARSER_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_MAIN_OBJS := $(TEST_SEMANTIC_MAIN_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_US11_OBJS := $(TEST_SEMANTIC_US11_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_US12_OBJS := $(TEST_SEMANTIC_US12_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_US13_OBJS := $(TEST_SEMANTIC_US13_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_INTEGRATION_OBJS := $(TEST_INTEGRATION_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)

# --- Targets ---
.PHONY: all test test_lexer test_parser test_semantic test_integration clean dirs

all: dirs $(BIN_DIR)/test_lexer.exe $(BIN_DIR)/test_parser.exe $(BIN_DIR)/test_semantic_main.exe $(BIN_DIR)/test_semantic_us11.exe $(BIN_DIR)/test_semantic_us12.exe $(BIN_DIR)/test_semantic_us13.exe $(BIN_DIR)/test_integration.exe
	@echo All test executables built successfully.

# Run all tests
test: test_lexer test_parser test_semantic test_integration

# Run lexer tests
test_lexer: $(BIN_DIR)/test_lexer.exe
	@echo --- Running Lexer Tests ---
	@./$(BIN_DIR)/test_lexer.exe
	@echo --- Lexer Tests Complete ---

# Run parser tests
test_parser: $(BIN_DIR)/test_parser.exe
	@echo --- Running Parser Tests ---
	@./$(BIN_DIR)/test_parser.exe
	@echo --- Parser Tests Complete ---

# Run all semantic tests
test_semantic: $(BIN_DIR)/test_semantic_main.exe $(BIN_DIR)/test_semantic_us11.exe $(BIN_DIR)/test_semantic_us12.exe $(BIN_DIR)/test_semantic_us13.exe
	@echo ========================================
	@echo Running ALL Semantic Analysis Tests
	@echo ========================================
	@echo
	@echo --- Main Semantic Tests ---
	@./$(BIN_DIR)/test_semantic_main.exe
	@echo
	@echo --- User Story \#11: Implicit Type Conversions ---
	@./$(BIN_DIR)/test_semantic_us11.exe
	@echo
	@echo --- User Story \#12: Pointer Arithmetic Validation ---
	@./$(BIN_DIR)/test_semantic_us12.exe
	@echo
	@echo --- User Story \#13: Struct Type Checking ---
	@./$(BIN_DIR)/test_semantic_us13.exe
	@echo
	@echo ========================================
	@echo All Semantic Tests Complete!
	@echo ========================================

# Run integration tests
test_integration: $(BIN_DIR)/test_integration.exe
	@echo ========================================
	@echo Running Integration Tests
	@echo ========================================
	@./$(BIN_DIR)/test_integration.exe
	@echo ========================================
	@echo Integration Tests Complete!
	@echo ========================================

# Create directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/tests
	@mkdir -p $(BIN_DIR)

# --- Linking ---
$(BIN_DIR)/test_lexer.exe: $(LEXER_OBJS) $(TEST_LEXER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(TEST_LEXER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_parser.exe: $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_semantic_main.exe: $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_MAIN_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_MAIN_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_semantic_us11.exe: $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_US11_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_US11_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_semantic_us12.exe: $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_US12_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_US12_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_semantic_us13.exe: $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_US13_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SEMANTIC_OBJS) $(AST_OBJS) $(TEST_SEMANTIC_US13_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_integration.exe: $(LEXER_OBJS) $(PARSER_OBJS) $(AST_OBJS) $(SEMANTIC_OBJS) $(TEST_INTEGRATION_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(AST_OBJS) $(SEMANTIC_OBJS) $(TEST_INTEGRATION_OBJS) -o $@
	@echo Linked $@.

# --- Compilation Rules ---
$(OBJ_DIR)/%.o: src/lexer/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/parser/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/AST/%.o: src/AST/%.cpp | dirs
	@mkdir -p $(OBJ_DIR)/AST
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/semantic/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Cleanup ---
clean:
	@echo Cleaning up build directories...
	@rm -rf build bin
	@echo Cleanup complete.
