#include "json_serializers.h"
#include "third_party/json.hpp"
#include <sstream>

using json = nlohmann::json;

// ============================================================================
// Token Serialization Implementation
// ============================================================================

/**
 * Convert a single token to a JSON object
 *
 * @param token The token to serialize
 * @return JSON object representing the token
 */
static json tokenToJson(const Token& token) {
    json j;

    // Serialize token type (use the existing token_type_to_string function)
    j["type"] = token_type_to_string(token.type);

    // Serialize token value (convert string_view to string)
    j["value"] = std::string(token.value);

    // Include processed_value only if it differs from value
    // This is relevant for string/char literals that have escape sequences
    if (!token.processed_value.empty() && token.processed_value != token.value) {
        j["processed_value"] = token.processed_value;
    }

    // Serialize location information
    j["location"] = {
        {"file", token.filename},
        {"line", token.line},
        {"column", token.column}
    };

    return j;
}

/**
 * Serialize a vector of tokens to JSON string
 *
 * Implementation notes:
 * - EOF_TOKEN is excluded from output for cleaner JSON
 * - Tokens are serialized in order with all metadata
 * - Output is pretty-printed with 2-space indentation
 *
 * @param tokens Vector of Token objects from lexer
 * @return JSON string representation
 */
std::string serializeTokensToJson(const std::vector<Token>& tokens) {
    json result;

    // Add metadata
    result["stage"] = "lexical_analysis";

    // Count tokens (excluding EOF_TOKEN)
    int tokenCount = 0;
    for (const auto& token : tokens) {
        if (token.type != TokenType::EOF_TOKEN) {
            tokenCount++;
        }
    }
    result["token_count"] = tokenCount;

    // Serialize tokens array
    json tokensArray = json::array();
    for (const auto& token : tokens) {
        // Skip EOF_TOKEN for cleaner output
        if (token.type != TokenType::EOF_TOKEN) {
            tokensArray.push_back(tokenToJson(token));
        }
    }
    result["tokens"] = tokensArray;

    // Convert to string with pretty printing (2-space indentation)
    return result.dump(2);
}
