CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -Iinclude

# Directories
SRC_DIR = src
BUILD_DIR = build/obj
BIN_DIR = bin

# Compiler executable
COMPILER_EXE = $(BIN_DIR)/mycc

# All object files for the compiler
OBJS = $(BUILD_DIR)/error_handler.o \
       $(BUILD_DIR)/lexer.o \
       $(BUILD_DIR)/token.o \
       $(BUILD_DIR)/parser.o \
       $(BUILD_DIR)/ast_printer.o \
       $(BUILD_DIR)/type.o \
       $(BUILD_DIR)/symbol_table.o \
       $(BUILD_DIR)/scope_manager.o \
       $(BUILD_DIR)/semantic_analyzer.o \
       $(BUILD_DIR)/ir.o \
       $(BUILD_DIR)/ir_codegen.o \
       $(BUILD_DIR)/ir_optimizer.o \
       $(BUILD_DIR)/codegen.o \
       $(BUILD_DIR)/compiler_driver.o \
       $(BUILD_DIR)/token_serializer.o \
       $(BUILD_DIR)/ast_serializer.o \
       $(BUILD_DIR)/hex_dump.o

.PHONY: all clean dirs dev-frontend dev-api dev build-frontend install-frontend install-api docker-build docker-shell

# Default target now builds in Docker
all: docker-build

# Docker build - builds compiler in Docker container
docker-build:
	@echo "========================================"
	@echo "Building compiler in Docker..."
	@echo "========================================"
	@docker-compose build --quiet
	@docker-compose run --rm compiler make native
	@echo "========================================"
	@echo "Build complete: $(COMPILER_EXE)"
	@echo "Creating wrapper script..."
	@echo "========================================"
	@echo '#!/bin/bash' > mycc
	@echo 'docker-compose run --rm compiler /app/bin/mycc "$$@"' >> mycc
	@chmod +x mycc
	@echo "✓ You can now run: ./mycc program.c --dump-hex output.hex -o program"

# Native build (runs inside Docker or locally)
native: dirs $(COMPILER_EXE)
	@echo "Native build complete"

# Development commands
dev: dev-api dev-frontend

dev-frontend:
	@echo "Starting React frontend development server..."
	@cd frontend && npm run dev

dev-api:
	@echo "Starting Flask API server..."
	@cd api && python3 app.py

build-frontend:
	@echo "Building React frontend for production..."
	@cd frontend && npm run build

install-frontend:
	@echo "Installing frontend dependencies..."
	@cd frontend && npm install

install-api:
	@echo "Installing API dependencies..."
	@cd api && python3 -m pip install -r requirements.txt

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Build compiler executable (requires main.cpp - to be created)
$(COMPILER_EXE): $(OBJS) $(BUILD_DIR)/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build library only (without main)
lib: dirs $(OBJS)
	@echo "Core library objects built."

# Error handling
$(BUILD_DIR)/error_handler.o: $(SRC_DIR)/error/error_handler.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Lexer
$(BUILD_DIR)/lexer.o: $(SRC_DIR)/lexer/lexer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/token.o: $(SRC_DIR)/lexer/token.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Parser
$(BUILD_DIR)/parser.o: $(SRC_DIR)/parser/parser.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# AST
$(BUILD_DIR)/ast_printer.o: $(SRC_DIR)/AST/ast_printer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Semantic analysis
$(BUILD_DIR)/type.o: $(SRC_DIR)/semantic/type.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/symbol_table.o: $(SRC_DIR)/semantic/symbol_table.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/scope_manager.o: $(SRC_DIR)/semantic/scope_manager.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/semantic_analyzer.o: $(SRC_DIR)/semantic/semantic_analyzer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# IR generation and optimization
$(BUILD_DIR)/ir.o: $(SRC_DIR)/ir/ir.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ir_codegen.o: $(SRC_DIR)/ir/ir_codegen.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ir_optimizer.o: $(SRC_DIR)/ir/ir_optimizer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Code generation
$(BUILD_DIR)/codegen.o: $(SRC_DIR)/codegen/codegen.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compiler driver
$(BUILD_DIR)/compiler_driver.o: $(SRC_DIR)/compiler/compiler_driver.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main entry point (to be created in src/main.cpp)
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Serializers
$(BUILD_DIR)/token_serializer.o: $(SRC_DIR)/serializers/token_serializer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/ast_serializer.o: $(SRC_DIR)/serializers/ast_serializer.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/hex_dump.o: $(SRC_DIR)/serializers/hex_dump.cpp | dirs
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@rm -f mycc
	@echo "Cleaned build directories"

# Docker shell - opens interactive shell in Docker container
docker-shell:
	@docker-compose run --rm compiler bash

# Docker dev - starts API and frontend in Docker
docker-dev:
	@docker-compose up
