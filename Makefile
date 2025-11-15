# --- Compiler and Flags ---
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g
INCLUDES := -Iinclude
TEST_LEXER_EXE := test_lexer.exe
TEST_PARSER_EXE := test_parser_identifier.exe

# --- Directories ---
OBJ_DIR := build/obj
BIN_DIR := bin

# --- Source Files ---
LEXER_SRCS := src/lexer/lexer.cpp src/lexer/token.cpp
PARSER_SRCS := src/parser/parser.cpp
TEST_LEXER_SRCS := test_lexer.cpp
TEST_PARSER_SRCS := test_parser_identifier.cpp
ALL_SRCS := $(LEXER_SRCS) $(PARSER_SRCS) $(TEST_LEXER_SRCS) $(TEST_PARSER_SRCS)

# --- Object Files ---
LEXER_OBJS := $(LEXER_SRCS:src/lexer/%.cpp=$(OBJ_DIR)/%.o)
PARSER_OBJS := $(PARSER_SRCS:src/parser/%.cpp=$(OBJ_DIR)/%.o)
TEST_LEXER_OBJS := $(TEST_LEXER_SRCS:%.cpp=$(OBJ_DIR)/%.o)
TEST_PARSER_OBJS := $(TEST_PARSER_SRCS:%.cpp=$(OBJ_DIR)/%.o)
ALL_OBJS := $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_LEXER_OBJS) $(TEST_PARSER_OBJS)

# --- Targets ---
.PHONY: all test test_lexer test_parser clean dirs

all: dirs $(BIN_DIR)/$(TEST_LEXER_EXE) $(BIN_DIR)/$(TEST_PARSER_EXE)
	@echo All test executables built successfully.

# Run all tests
test: test_lexer test_parser

# Run lexer tests
test_lexer: $(BIN_DIR)/$(TEST_LEXER_EXE)
	@echo --- Running Lexer Test ---
	@./$(BIN_DIR)/$(TEST_LEXER_EXE)
	@echo --- Lexer Test Complete ---

# Run parser tests
test_parser: $(BIN_DIR)/$(TEST_PARSER_EXE)
	@echo --- Running Parser Identifier Test ---
	@./$(BIN_DIR)/$(TEST_PARSER_EXE)
	@echo --- Parser Test Complete ---

# Uses Windows-native commands for directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# --- Linking ---
$(BIN_DIR)/$(TEST_LEXER_EXE): $(LEXER_OBJS) $(TEST_LEXER_OBJS)
	$(CXX) $(CXXFLAGS) $(LEXER_OBJS) $(TEST_LEXER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/$(TEST_PARSER_EXE): $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_OBJS)
	$(CXX) $(CXXFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_OBJS) -o $@
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

# --- Cleanup ---
# Uses Windows-native commands for cleanup
clean:
	@echo Cleaning up build directories...
	@rm -rf build bin
	@echo Cleanup complete.
