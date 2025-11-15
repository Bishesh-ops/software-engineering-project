# --- Compiler and Flags ---
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g
INCLUDES := -Iinclude
TEST_LEXER_EXE := test_lexer.exe
TEST_PARSER_IDENTIFIER_EXE := test_parser_identifier.exe
TEST_PARSER_BINARY_EXE := test_parser_binary.exe

# --- Directories ---
OBJ_DIR := build/obj
BIN_DIR := bin

# --- Source Files ---
LEXER_SRCS := src/lexer/lexer.cpp src/lexer/token.cpp
PARSER_SRCS := src/parser/parser.cpp
TEST_LEXER_SRCS := test_lexer.cpp
TEST_PARSER_IDENTIFIER_SRCS := test_parser_identifier.cpp
TEST_PARSER_BINARY_SRCS := test_parser_binary.cpp
ALL_SRCS := $(LEXER_SRCS) $(PARSER_SRCS) $(TEST_LEXER_SRCS) $(TEST_PARSER_IDENTIFIER_SRCS) $(TEST_PARSER_BINARY_SRCS)

# --- Object Files ---
LEXER_OBJS := $(LEXER_SRCS:src/lexer/%.cpp=$(OBJ_DIR)/%.o)
PARSER_OBJS := $(PARSER_SRCS:src/parser/%.cpp=$(OBJ_DIR)/%.o)
TEST_LEXER_OBJS := $(TEST_LEXER_SRCS:%.cpp=$(OBJ_DIR)/%.o)
TEST_PARSER_IDENTIFIER_OBJS := $(TEST_PARSER_IDENTIFIER_SRCS:%.cpp=$(OBJ_DIR)/%.o)
TEST_PARSER_BINARY_OBJS := $(TEST_PARSER_BINARY_SRCS:%.cpp=$(OBJ_DIR)/%.o)
ALL_OBJS := $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_LEXER_OBJS) $(TEST_PARSER_IDENTIFIER_OBJS) $(TEST_PARSER_BINARY_OBJS)

# --- Targets ---
.PHONY: all test test_lexer test_parser_identifier test_parser_binary test_parser clean dirs

all: dirs $(BIN_DIR)/$(TEST_LEXER_EXE) $(BIN_DIR)/$(TEST_PARSER_IDENTIFIER_EXE) $(BIN_DIR)/$(TEST_PARSER_BINARY_EXE)
	@echo All test executables built successfully.

# Run all tests
test: test_lexer test_parser

# Run all parser tests
test_parser: test_parser_identifier test_parser_binary

# Run lexer tests
test_lexer: $(BIN_DIR)/$(TEST_LEXER_EXE)
	@echo --- Running Lexer Test ---
	@./$(BIN_DIR)/$(TEST_LEXER_EXE)
	@echo --- Lexer Test Complete ---

# Run parser identifier tests
test_parser_identifier: $(BIN_DIR)/$(TEST_PARSER_IDENTIFIER_EXE)
	@echo --- Running Parser Identifier Test ---
	@./$(BIN_DIR)/$(TEST_PARSER_IDENTIFIER_EXE)
	@echo --- Parser Identifier Test Complete ---

# Run parser binary expression tests
test_parser_binary: $(BIN_DIR)/$(TEST_PARSER_BINARY_EXE)
	@echo --- Running Parser Binary Expression Test ---
	@./$(BIN_DIR)/$(TEST_PARSER_BINARY_EXE)
	@echo --- Parser Binary Test Complete ---

# Uses Windows-native commands for directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# --- Linking ---
$(BIN_DIR)/$(TEST_LEXER_EXE): $(LEXER_OBJS) $(TEST_LEXER_OBJS)
	$(CXX) $(CXXFLAGS) $(LEXER_OBJS) $(TEST_LEXER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/$(TEST_PARSER_IDENTIFIER_EXE): $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_IDENTIFIER_OBJS)
	$(CXX) $(CXXFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_IDENTIFIER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/$(TEST_PARSER_BINARY_EXE): $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_BINARY_OBJS)
	$(CXX) $(CXXFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_BINARY_OBJS) -o $@
	@echo Linked $@.

# --- Compilation Rules ---
# Compile lexer source files
$(OBJ_DIR)/%.o: src/lexer/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile parser source files
$(OBJ_DIR)/%.o: src/parser/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile test source files
$(OBJ_DIR)/test_lexer.o: test_lexer.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_parser_identifier.o: test_parser_identifier.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_parser_binary.o: test_parser_binary.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Cleanup ---
# Uses Windows-native commands for cleanup
clean:
	@echo Cleaning up build directories...
	@rm -rf build bin
	@echo Cleanup complete.
