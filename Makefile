CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build/obj
BIN_DIR = bin

# Source files
ERROR_SRC = $(SRC_DIR)/error/error_handler.cpp
LEXER_SRCS = $(SRC_DIR)/lexer/lexer.cpp $(SRC_DIR)/lexer/token.cpp
PARSER_SRC = $(SRC_DIR)/parser/parser.cpp
SEMANTIC_SRCS = $(SRC_DIR)/semantic/type.cpp $(SRC_DIR)/semantic/symbol_table.cpp $(SRC_DIR)/semantic/scope_manager.cpp $(SRC_DIR)/semantic/semantic_analyzer.cpp
AST_SRC = $(SRC_DIR)/AST/ast_printer.cpp

# Object files
ERROR_OBJ = $(BUILD_DIR)/error_handler.o
LEXER_OBJS = $(BUILD_DIR)/lexer.o $(BUILD_DIR)/token.o
PARSER_OBJ = $(BUILD_DIR)/parser.o
SEMANTIC_OBJS = $(BUILD_DIR)/type.o $(BUILD_DIR)/symbol_table.o $(BUILD_DIR)/scope_manager.o $(BUILD_DIR)/semantic_analyzer.o
AST_OBJ = $(BUILD_DIR)/ast_printer.o

# Test
TEST_SRC = tests/temp_test.cpp
TEST_OBJ = $(BUILD_DIR)/temp_test.o
TEST_EXE = $(BIN_DIR)/temp_test.exe

.PHONY: all clean dirs test

all: dirs $(TEST_EXE)

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Build test executable
$(TEST_EXE): $(ERROR_OBJ) $(LEXER_OBJS) $(PARSER_OBJ) $(SEMANTIC_OBJS) $(AST_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Built: $@"

# Compile object files
$(BUILD_DIR)/error_handler.o: $(ERROR_SRC) | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/lexer.o: $(SRC_DIR)/lexer/lexer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/token.o: $(SRC_DIR)/lexer/token.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/parser.o: $(PARSER_SRC) | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/type.o: $(SRC_DIR)/semantic/type.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/symbol_table.o: $(SRC_DIR)/semantic/symbol_table.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/scope_manager.o: $(SRC_DIR)/semantic/scope_manager.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/semantic_analyzer.o: $(SRC_DIR)/semantic/semantic_analyzer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ast_printer.o: $(AST_SRC) | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/temp_test.o: $(TEST_SRC) | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run test
test: $(TEST_EXE)
	@echo "========================================"
	@echo "Running Temporary Test"
	@echo "========================================"
	@./$(TEST_EXE)

# Clean
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaned build directories"
