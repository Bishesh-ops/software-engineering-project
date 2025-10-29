# --- Compiler and Flags ---
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g
INCLUDES := -Iinclude
TEST_EXE := test_lexer.exe

# --- Directories ---
OBJ_DIR := build/obj
BIN_DIR := bin

# --- Source Files ---
LEXER_SRCS := src/lexer/lexer.cpp src/lexer/token.cpp
TEST_SRCS := test_lexer.cpp
ALL_SRCS := $(LEXER_SRCS) $(TEST_SRCS)

# --- Object Files ---
LEXER_OBJS := $(LEXER_SRCS:src/lexer/%.cpp=$(OBJ_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(OBJ_DIR)/%.o)
ALL_OBJS := $(LEXER_OBJS) $(TEST_OBJS)

# --- Targets ---
.PHONY: all test clean dirs

all: dirs $(BIN_DIR)/$(TEST_EXE)
	@echo Lexer Test executable built successfully.

# UPDATED: Removed PowerShell timing command, just runs the executable
test: all
	@echo --- Running Lexer Test ---
	@./$(BIN_DIR)/$(TEST_EXE)
	@echo --- Test Complete ---

# Uses Windows-native commands for directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# --- Linking ---
$(BIN_DIR)/$(TEST_EXE): $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) $(ALL_OBJS) -o $@
	@echo Linked $@.

# --- Compilation Rules ---
# Compile lexer source files
$(OBJ_DIR)/%.o: src/lexer/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile test source file (only test_lexer.cpp)
$(OBJ_DIR)/test_lexer.o: test_lexer.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Cleanup ---
# Uses Windows-native commands for cleanup
clean:
	@echo Cleaning up build directories...
	@if exist "build" rmdir /s /q "build"
	@if exist "bin" rmdir /s /q "bin"
	@echo Cleanup complete.