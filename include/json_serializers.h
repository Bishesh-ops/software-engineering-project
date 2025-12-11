#ifndef JSON_SERIALIZERS_H
#define JSON_SERIALIZERS_H

#include <string>
#include <vector>
#include <memory>
#include "lexer.h"
#include "ast.h"

// ============================================================================
// JSON Serializers - Convert Compiler Data Structures to JSON
// ============================================================================
// This module provides serialization functions for converting internal
// compiler data structures (tokens, AST, IR, etc.) into JSON format.
//
// Usage:
//   std::vector<Token> tokens = lexer.lexAll();
//   std::string json = serializeTokensToJson(tokens);
//   // Write json to file or send over network
//
// Output Format:
//   {
//     "stage": "lexical_analysis",
//     "token_count": N,
//     "tokens": [
//       {
//         "type": "KW_INT",
//         "value": "int",
//         "location": {
//           "file": "test.c",
//           "line": 1,
//           "column": 1
//         }
//       },
//       ...
//     ]
//   }

// ============================================================================
// Token Serialization
// ============================================================================

/**
 * Serialize a vector of tokens to JSON string
 *
 * @param tokens Vector of Token objects from the lexer
 * @return JSON string representation of the tokens
 *
 * The output JSON contains:
 * - stage: "lexical_analysis"
 * - token_count: Total number of tokens (excluding EOF_TOKEN)
 * - tokens: Array of token objects with type, value, and location
 *
 * Token types are serialized using their enum name (e.g., "KW_INT", "IDENTIFIER")
 * EOF_TOKEN is excluded from the output for cleaner JSON
 */
std::string serializeTokensToJson(const std::vector<Token>& tokens);

// ============================================================================
// AST Serialization
// ============================================================================

/**
 * Serialize an Abstract Syntax Tree to JSON string
 *
 * @param ast Vector of top-level Declaration nodes (the program)
 * @return JSON string representation of the AST
 *
 * The output JSON contains:
 * - stage: "parsing"
 * - declaration_count: Number of top-level declarations
 * - declarations: Array of AST nodes recursively serialized
 *
 * Uses the Visitor pattern to recursively traverse the AST.
 * Each node is serialized with:
 * - node_type: The type of AST node (e.g., "FUNCTION_DECL", "BINARY_EXPR")
 * - location: Source file location {file, line, column}
 * - Node-specific fields (e.g., operator for BinaryExpr, name for IdentifierExpr)
 * - Child nodes recursively serialized
 */
std::string serializeAstToJson(const std::vector<std::unique_ptr<Declaration>>& ast);

// ============================================================================
// Binary Hex Dump Utility
// ============================================================================

/**
 * Generate a hexadecimal dump of a binary file
 *
 * @param executablePath Path to the binary file to dump
 * @return Hex dump string formatted as space-separated bytes (16 bytes per line)
 *
 * Output format:
 *   4D 5A 90 00 03 00 00 00 04 00 00 00 FF FF 00 00
 *   48 89 E5 48 83 EC 10 C7 45 FC 2A 00 00 00 8B 45
 *   ...
 *
 * Each line contains 16 bytes represented as uppercase hexadecimal.
 * Returns empty string if file cannot be read.
 *
 * This is useful for:
 * - Visualizing final executable binary content
 * - Debugging linking issues
 * - Understanding executable structure (ELF/Mach-O headers, code sections, etc.)
 */
std::string generateHexDump(const std::string& executablePath);

#endif // JSON_SERIALIZERS_H
