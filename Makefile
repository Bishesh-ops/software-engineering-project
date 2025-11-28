# --- Compiler and Flags ---
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -g
# macOS uses LLVM linker, Linux/Windows use GNU ld
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
    LDFLAGS :=
else ifneq (,$(findstring MINGW,$(UNAME)))
    LDFLAGS := -Xlinker --allow-multiple-definition
else ifneq (,$(findstring MSYS,$(UNAME)))
    LDFLAGS := -Xlinker --allow-multiple-definition
else
    LDFLAGS := -Wl,--allow-multiple-definition
endif
INCLUDES := -Iinclude

# --- Directories ---
OBJ_DIR := build/obj
BIN_DIR := bin

# --- Source Files ---
ERROR_SRCS := src/error/error_handler.cpp
LEXER_SRCS := src/lexer/lexer.cpp src/lexer/token.cpp
PARSER_SRCS := src/parser/parser.cpp
AST_SRCS := src/AST/ast_printer.cpp
SEMANTIC_SRCS := src/semantic/type.cpp src/semantic/symbol_table.cpp src/semantic/scope_manager.cpp src/semantic/semantic_analyzer.cpp
IR_SRCS := src/ir/ir.cpp src/ir/ir_codegen.cpp src/ir/ir_optimizer.cpp
CODEGEN_SRCS := src/codegen/codegen.cpp
TEST_LEXER_SRCS := tests/test_lexer.cpp
TEST_PARSER_SRCS := tests/test_parser.cpp
TEST_SEMANTIC_MAIN_SRCS := tests/test_semantic_main.cpp
TEST_SEMANTIC_US11_SRCS := tests/test_semantic_us11_implicit_conversions.cpp
TEST_SEMANTIC_US12_SRCS := tests/test_semantic_us12_pointer_arithmetic.cpp
TEST_SEMANTIC_US13_SRCS := tests/test_semantic_us13_struct_checking.cpp
TEST_INTEGRATION_SRCS := tests/test_integration.cpp
TEST_IR_SRCS := tests/test_ir.cpp
TEST_TEMP_GEN_SRCS := tests/test_temp_generator.cpp
TEST_CONSTANT_FOLDING_SRCS := tests/test_constant_folding.cpp
TEST_DEAD_CODE_ELIM_SRCS := tests/test_dead_code_elimination.cpp
TEST_CSE_SRCS := tests/test_cse.cpp
TEST_CODEGEN_SRCS := tests/test_codegen.cpp
TEST_EXTERNAL_SRCS := tests/test_external_calls.cpp
TEST_PEEPHOLE_SRCS := tests/test_peephole_optimization.cpp
TEST_OUTPUT_VALIDATION_SRCS := tests/test_output_validation.cpp
TEST_ERROR_HANDLER_SRCS := tests/test_error_handler.cpp
EXAMPLE_EXPR_LOWERING_SRCS := examples/expression_lowering_example.cpp
EXAMPLE_ASSIGN_LOWERING_SRCS := examples/assignment_lowering_example.cpp
EXAMPLE_IF_LOWERING_SRCS := examples/if_lowering_example.cpp
EXAMPLE_WHILE_LOWERING_SRCS := examples/while_lowering_example.cpp
EXAMPLE_FOR_LOWERING_SRCS := examples/for_lowering_example.cpp
EXAMPLE_CALL_LOWERING_SRCS := examples/call_lowering_example.cpp
EXAMPLE_FUNC_LOWERING_SRCS := examples/function_lowering_example.cpp
EXAMPLE_MEMORY_LOWERING_SRCS := examples/memory_lowering_example.cpp
EXAMPLE_IR_PRINTER_SRCS := examples/ir_printer_example.cpp
EXAMPLE_CONSTANT_FOLDING_SRCS := examples/constant_folding_example.cpp
EXAMPLE_DEAD_CODE_ELIM_SRCS := examples/dead_code_elimination_example.cpp
EXAMPLE_CSE_SRCS := examples/cse_example.cpp
EXAMPLE_CODEGEN_SRCS := examples/codegen_example.cpp
EXAMPLE_EXTERNAL_PRINTF_SRCS := examples/external_printf_example.cpp
EXAMPLE_EXTERNAL_MATH_SRCS := examples/external_math_example.cpp
EXAMPLE_DEBUG_SYMBOLS_SRCS := examples/debug_symbols_example.cpp

# --- Object Files ---
ERROR_OBJS := $(ERROR_SRCS:src/error/%.cpp=$(OBJ_DIR)/%.o)
LEXER_OBJS := $(LEXER_SRCS:src/lexer/%.cpp=$(OBJ_DIR)/%.o)
PARSER_OBJS := $(PARSER_SRCS:src/parser/%.cpp=$(OBJ_DIR)/%.o)
AST_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(AST_SRCS))
SEMANTIC_OBJS := $(SEMANTIC_SRCS:src/semantic/%.cpp=$(OBJ_DIR)/%.o)
IR_OBJS := $(IR_SRCS:src/ir/%.cpp=$(OBJ_DIR)/%.o)
CODEGEN_OBJS := $(CODEGEN_SRCS:src/codegen/%.cpp=$(OBJ_DIR)/%.o)
TEST_LEXER_OBJS := $(TEST_LEXER_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_PARSER_OBJS := $(TEST_PARSER_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_MAIN_OBJS := $(TEST_SEMANTIC_MAIN_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_US11_OBJS := $(TEST_SEMANTIC_US11_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_US12_OBJS := $(TEST_SEMANTIC_US12_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_SEMANTIC_US13_OBJS := $(TEST_SEMANTIC_US13_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_INTEGRATION_OBJS := $(TEST_INTEGRATION_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_IR_OBJS := $(TEST_IR_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_TEMP_GEN_OBJS := $(TEST_TEMP_GEN_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_CONSTANT_FOLDING_OBJS := $(TEST_CONSTANT_FOLDING_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_DEAD_CODE_ELIM_OBJS := $(TEST_DEAD_CODE_ELIM_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_CSE_OBJS := $(TEST_CSE_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_CODEGEN_OBJS := $(TEST_CODEGEN_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_EXTERNAL_OBJS := $(TEST_EXTERNAL_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_PEEPHOLE_OBJS := $(TEST_PEEPHOLE_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_OUTPUT_VALIDATION_OBJS := $(TEST_OUTPUT_VALIDATION_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
TEST_ERROR_HANDLER_OBJS := $(TEST_ERROR_HANDLER_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)
EXAMPLE_EXPR_LOWERING_OBJS := $(EXAMPLE_EXPR_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_ASSIGN_LOWERING_OBJS := $(EXAMPLE_ASSIGN_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_IF_LOWERING_OBJS := $(EXAMPLE_IF_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_WHILE_LOWERING_OBJS := $(EXAMPLE_WHILE_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_FOR_LOWERING_OBJS := $(EXAMPLE_FOR_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_CALL_LOWERING_OBJS := $(EXAMPLE_CALL_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_FUNC_LOWERING_OBJS := $(EXAMPLE_FUNC_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_MEMORY_LOWERING_OBJS := $(EXAMPLE_MEMORY_LOWERING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_IR_PRINTER_OBJS := $(EXAMPLE_IR_PRINTER_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_CONSTANT_FOLDING_OBJS := $(EXAMPLE_CONSTANT_FOLDING_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_DEAD_CODE_ELIM_OBJS := $(EXAMPLE_DEAD_CODE_ELIM_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_CSE_OBJS := $(EXAMPLE_CSE_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_CODEGEN_OBJS := $(EXAMPLE_CODEGEN_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_EXTERNAL_PRINTF_OBJS := $(EXAMPLE_EXTERNAL_PRINTF_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_EXTERNAL_MATH_OBJS := $(EXAMPLE_EXTERNAL_MATH_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)
EXAMPLE_DEBUG_SYMBOLS_OBJS := $(EXAMPLE_DEBUG_SYMBOLS_SRCS:examples/%.cpp=$(OBJ_DIR)/examples/%.o)

# --- Targets ---
.PHONY: all test test_error_handler test_lexer test_parser test_semantic test_integration test_ir test_temp_gen test_constant_folding test_dead_code_elim test_cse test_codegen test_external test_peephole test_output_validation example_expr_lowering example_assign_lowering example_if_lowering example_while_lowering example_for_lowering example_call_lowering example_func_lowering example_memory_lowering example_ir_printer example_constant_folding example_dead_code_elim example_cse example_codegen example_external_printf example_external_math example_debug_symbols clean dirs

all: dirs $(BIN_DIR)/test_error_handler.exe $(BIN_DIR)/test_lexer.exe $(BIN_DIR)/test_parser.exe $(BIN_DIR)/test_semantic_main.exe $(BIN_DIR)/test_semantic_us11.exe $(BIN_DIR)/test_semantic_us12.exe $(BIN_DIR)/test_semantic_us13.exe $(BIN_DIR)/test_integration.exe $(BIN_DIR)/test_ir.exe $(BIN_DIR)/test_temp_generator.exe $(BIN_DIR)/test_constant_folding.exe $(BIN_DIR)/test_dead_code_elimination.exe $(BIN_DIR)/test_cse.exe $(BIN_DIR)/test_codegen.exe $(BIN_DIR)/test_external_calls.exe $(BIN_DIR)/test_peephole_optimization.exe $(BIN_DIR)/test_output_validation.exe $(BIN_DIR)/expression_lowering_example.exe $(BIN_DIR)/assignment_lowering_example.exe $(BIN_DIR)/if_lowering_example.exe $(BIN_DIR)/while_lowering_example.exe $(BIN_DIR)/for_lowering_example.exe $(BIN_DIR)/call_lowering_example.exe $(BIN_DIR)/function_lowering_example.exe $(BIN_DIR)/memory_lowering_example.exe $(BIN_DIR)/ir_printer_example.exe $(BIN_DIR)/constant_folding_example.exe $(BIN_DIR)/dead_code_elimination_example.exe $(BIN_DIR)/cse_example.exe $(BIN_DIR)/codegen_example.exe $(BIN_DIR)/external_printf_example.exe $(BIN_DIR)/external_math_example.exe $(BIN_DIR)/debug_symbols_example.exe
	@echo All test executables built successfully.

# Run all tests
test: test_error_handler test_lexer test_parser test_semantic test_integration test_ir test_temp_gen test_constant_folding test_dead_code_elim test_cse test_codegen test_external test_peephole test_output_validation

# Run error handler tests
test_error_handler: $(BIN_DIR)/test_error_handler.exe
	@echo ========================================
	@echo Running Error Handler Tests
	@echo ========================================
	@./$(BIN_DIR)/test_error_handler.exe
	@echo ========================================
	@echo Error Handler Tests Complete!
	@echo ========================================

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

# Run IR tests
test_ir: $(BIN_DIR)/test_ir.exe
	@echo ========================================
	@echo Running IR Instruction Tests
	@echo ========================================
	@./$(BIN_DIR)/test_ir.exe
	@echo ========================================
	@echo IR Tests Complete!
	@echo ========================================

# Run Temp Generator tests
test_temp_gen: $(BIN_DIR)/test_temp_generator.exe
	@echo ========================================
	@echo Running Temporary Variable Generator Tests
	@echo ========================================
	@./$(BIN_DIR)/test_temp_generator.exe
	@echo ========================================
	@echo Temp Generator Tests Complete!
	@echo ========================================

# Run Constant Folding tests
test_constant_folding: $(BIN_DIR)/test_constant_folding.exe
	@echo ========================================
	@echo Running Constant Folding Optimization Tests
	@echo ========================================
	@./$(BIN_DIR)/test_constant_folding.exe
	@echo ========================================
	@echo Constant Folding Tests Complete!
	@echo ========================================

# Run Dead Code Elimination tests
test_dead_code_elim: $(BIN_DIR)/test_dead_code_elimination.exe
	@echo ========================================
	@echo Running Dead Code Elimination Optimization Tests
	@echo ========================================
	@./$(BIN_DIR)/test_dead_code_elimination.exe
	@echo ========================================
	@echo Dead Code Elimination Tests Complete!
	@echo ========================================

# Run Common Subexpression Elimination tests
test_cse: $(BIN_DIR)/test_cse.exe
	@echo ========================================
	@echo Running Common Subexpression Elimination Tests
	@echo ========================================
	@./$(BIN_DIR)/test_cse.exe
	@echo ========================================
	@echo CSE Tests Complete!
	@echo ========================================

# Run Code Generation tests
test_codegen: $(BIN_DIR)/test_codegen.exe
	@echo ========================================
	@echo Running x86-64 Code Generation Tests
	@echo ========================================
	@./$(BIN_DIR)/test_codegen.exe
	@echo ========================================
	@echo Code Generation Tests Complete!
	@echo ========================================

# Run External Library Integration tests
test_external: $(BIN_DIR)/test_external_calls.exe
	@echo ========================================
	@echo Running External Library Integration Tests
	@echo ========================================
	@./$(BIN_DIR)/test_external_calls.exe
	@echo ========================================
	@echo External Library Tests Complete!
	@echo ========================================

# Run Peephole Optimization tests
test_peephole: $(BIN_DIR)/test_peephole_optimization.exe
	@echo ========================================
	@echo Running Peephole Optimization Tests
	@echo ========================================
	@./$(BIN_DIR)/test_peephole_optimization.exe
	@echo ========================================
	@echo Peephole Optimization Tests Complete!
	@echo ========================================

# Run Output Validation tests
test_output_validation: $(BIN_DIR)/test_output_validation.exe
	@echo ========================================
	@echo Running Output Validation Tests
	@echo ========================================
	@./$(BIN_DIR)/test_output_validation.exe
	@echo ========================================
	@echo Output Validation Tests Complete!
	@echo ========================================

# Run Expression Lowering Example
example_expr_lowering: $(BIN_DIR)/expression_lowering_example.exe
	@echo ========================================
	@echo Running Expression Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/expression_lowering_example.exe
	@echo ========================================
	@echo Expression Lowering Example Complete!
	@echo ========================================

# Run Assignment Lowering Example
example_assign_lowering: $(BIN_DIR)/assignment_lowering_example.exe
	@echo ========================================
	@echo Running Assignment Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/assignment_lowering_example.exe
	@echo ========================================
	@echo Assignment Lowering Example Complete!
	@echo ========================================

# Run If Statement Lowering Example
example_if_lowering: $(BIN_DIR)/if_lowering_example.exe
	@echo ========================================
	@echo Running If Statement Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/if_lowering_example.exe
	@echo ========================================
	@echo If Statement Lowering Example Complete!
	@echo ========================================

# Run While Loop Lowering Example
example_while_lowering: $(BIN_DIR)/while_lowering_example.exe
	@echo ========================================
	@echo Running While Loop Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/while_lowering_example.exe
	@echo ========================================
	@echo While Loop Lowering Example Complete!
	@echo ========================================

# Run For Loop Lowering Example
example_for_lowering: $(BIN_DIR)/for_lowering_example.exe
	@echo ========================================
	@echo Running For Loop Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/for_lowering_example.exe
	@echo ========================================
	@echo For Loop Lowering Example Complete!
	@echo ========================================

# Run Function Call Lowering Example
example_call_lowering: $(BIN_DIR)/call_lowering_example.exe
	@echo ========================================
	@echo Running Function Call Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/call_lowering_example.exe
	@echo ========================================
	@echo Function Call Lowering Example Complete!
	@echo ========================================

# Run Function Definition Lowering Example
example_func_lowering: $(BIN_DIR)/function_lowering_example.exe
	@echo ========================================
	@echo Running Function Definition Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/function_lowering_example.exe
	@echo ========================================
	@echo Function Definition Lowering Example Complete!
	@echo ========================================

# Run Memory Access Lowering Example
example_memory_lowering: $(BIN_DIR)/memory_lowering_example.exe
	@echo ========================================
	@echo Running Memory Access Lowering Example
	@echo ========================================
	@./$(BIN_DIR)/memory_lowering_example.exe
	@echo ========================================
	@echo Memory Access Lowering Example Complete!
	@echo ========================================

# Run IR Printer Example
example_ir_printer: $(BIN_DIR)/ir_printer_example.exe
	@echo ========================================
	@echo Running IR Printer Debug Example
	@echo ========================================
	@./$(BIN_DIR)/ir_printer_example.exe
	@echo ========================================
	@echo IR Printer Example Complete!
	@echo ========================================

# Create directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/tests
	@mkdir -p $(OBJ_DIR)/examples
	@mkdir -p $(BIN_DIR)

# --- Linking ---
$(BIN_DIR)/test_error_handler.exe: $(ERROR_OBJS) $(TEST_ERROR_HANDLER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(ERROR_OBJS) $(TEST_ERROR_HANDLER_OBJS) -o $@
	@echo Linked $@.

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

$(BIN_DIR)/test_integration.exe: $(LEXER_OBJS) $(PARSER_OBJS) $(AST_OBJS) $(SEMANTIC_OBJS) $(IR_OBJS) $(TEST_INTEGRATION_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(AST_OBJS) $(SEMANTIC_OBJS) $(IR_OBJS) $(TEST_INTEGRATION_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_ir.exe: $(IR_OBJS) $(TEST_IR_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(TEST_IR_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_temp_generator.exe: $(IR_OBJS) $(TEST_TEMP_GEN_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(TEST_TEMP_GEN_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_constant_folding.exe: $(IR_OBJS) $(TEST_CONSTANT_FOLDING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(TEST_CONSTANT_FOLDING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_dead_code_elimination.exe: $(IR_OBJS) $(TEST_DEAD_CODE_ELIM_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(TEST_DEAD_CODE_ELIM_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_cse.exe: $(IR_OBJS) $(TEST_CSE_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(TEST_CSE_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_codegen.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_CODEGEN_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_CODEGEN_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/expression_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_EXPR_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_EXPR_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/assignment_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_ASSIGN_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_ASSIGN_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/if_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_IF_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_IF_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/while_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_WHILE_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_WHILE_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/for_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_FOR_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_FOR_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/call_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_CALL_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_CALL_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/function_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_FUNC_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_FUNC_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/memory_lowering_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_MEMORY_LOWERING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_MEMORY_LOWERING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/ir_printer_example.exe: $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_IR_PRINTER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(AST_OBJS) $(EXAMPLE_IR_PRINTER_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/constant_folding_example.exe: $(IR_OBJS) $(EXAMPLE_CONSTANT_FOLDING_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(EXAMPLE_CONSTANT_FOLDING_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/dead_code_elimination_example.exe: $(IR_OBJS) $(EXAMPLE_DEAD_CODE_ELIM_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(EXAMPLE_DEAD_CODE_ELIM_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/cse_example.exe: $(IR_OBJS) $(EXAMPLE_CSE_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(EXAMPLE_CSE_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/codegen_example.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_CODEGEN_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_CODEGEN_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_external_calls.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_EXTERNAL_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_EXTERNAL_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_peephole_optimization.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_PEEPHOLE_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_PEEPHOLE_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/test_output_validation.exe: $(LEXER_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_OUTPUT_VALIDATION_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LEXER_OBJS) $(PARSER_OBJS) $(SEMANTIC_OBJS) $(IR_OBJS) $(CODEGEN_OBJS) $(TEST_OUTPUT_VALIDATION_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/external_printf_example.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_EXTERNAL_PRINTF_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_EXTERNAL_PRINTF_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/external_math_example.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_EXTERNAL_MATH_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_EXTERNAL_MATH_OBJS) -o $@
	@echo Linked $@.

$(BIN_DIR)/debug_symbols_example.exe: $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_DEBUG_SYMBOLS_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(IR_OBJS) $(CODEGEN_OBJS) $(EXAMPLE_DEBUG_SYMBOLS_OBJS) -o $@
	@echo Linked $@.

# --- Compilation Rules ---
$(OBJ_DIR)/%.o: src/error/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/lexer/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/parser/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/AST/%.o: src/AST/%.cpp | dirs
	@mkdir -p $(OBJ_DIR)/AST
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/semantic/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/ir/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: src/codegen/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/examples/%.o: examples/%.cpp | dirs
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# --- Cleanup ---
clean:
	@echo Cleaning up build directories...
	@rm -rf build bin
	@echo Cleanup complete.
