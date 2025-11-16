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
TEST_LEXER_SRCS := test_lexer.cpp
TEST_PARSER_SRCS := test_parser.cpp

# --- Object Files ---
LEXER_OBJS := $(LEXER_SRCS:src/lexer/%.cpp=$(OBJ_DIR)/%.o)
PARSER_OBJS := $(PARSER_SRCS:src/parser/%.cpp=$(OBJ_DIR)/%.o)
TEST_LEXER_OBJS := $(TEST_LEXER_SRCS:%.cpp=$(OBJ_DIR)/%.o)
TEST_PARSER_OBJS := $(TEST_PARSER_SRCS:%.cpp=$(OBJ_DIR)/%.o)

# --- Targets ---
.PHONY: all test test_lexer test_parser clean dirs

all: dirs $(BIN_DIR)/test_lexer.exe $(BIN_DIR)/test_parser.exe
	@echo All test executables built successfully.

# Run all tests
test: test_lexer test_parser

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

# Create directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# --- Linking ---
$(BIN_DIR)/test_lexer.exe: $(LEXER_OBJS) $(TEST_LEXER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(TEST_LEXER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_parser.exe: $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(TEST_PARSER_OBJS) -o $@
	@echo Linked $@.

# --- Compilation Rules ---
$(OBJ_DIR)/%.o: src/lexer/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/parser/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_lexer.o: test_lexer.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_parser.o: test_parser.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Cleanup ---
clean:
	@echo Cleaning up build directories...
	@rm -rf build bin
	@echo Cleanup complete.
