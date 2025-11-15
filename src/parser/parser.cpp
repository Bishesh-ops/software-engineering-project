#include "../../include/parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(Lexer &lexer) : lexer_(lexer), current_token_(lexer_.getNextToken())
{
}

// ============================================================================
// Token Management
// ============================================================================

void Parser::advance()
{
    current_token_ = lexer_.getNextToken();
}

bool Parser::check(TokenType type) const
{
    return current_token_.type == type;
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string &error_message)
{
    if (check(type))
    {
        Token token = current_token_;
        advance();
        return token;
    }
    reportError(error_message);
    throw std::runtime_error(error_message);
}

// ============================================================================
// Error Handling
// ============================================================================

void Parser::reportError(const std::string &message)
{
    std::cerr << currentLocation().toString() << ": Error: " << message << std::endl;
}

SourceLocation Parser::currentLocation() const
{
    return SourceLocation(current_token_.filename, current_token_.line, current_token_.column);
}

// ============================================================================
// Expression Parsing
// ============================================================================

std::unique_ptr<Expression> Parser::parseExpression()
{
    // For now, just parse primary expressions
    // This will be extended with operator precedence parsing later
    return parsePrimaryExpression();
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression()
{
    // Primary expressions:
    // - Identifiers (variable/function names)
    // - Literals (numbers, strings, chars)
    // - Parenthesized expressions (future)
    // - etc.

    switch (current_token_.type)
    {
    case TokenType::IDENTIFIER:
        return parseIdentifier();

    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::STRING_LITERAL:
    case TokenType::CHAR_LITERAL:
        return parseLiteral();

    default:
        reportError("Expected expression, got " + token_type_to_string(current_token_.type));
        throw std::runtime_error("Unexpected token in primary expression");
    }
}

// ============================================================================
// Identifier Parsing (USER STORY #2)
// ============================================================================

std::unique_ptr<Expression> Parser::parseIdentifier()
{
    // Accept: Creates IdentifierNode with variable name
    // Accept: Does NOT validate existence (that's semantic analysis later)

    Token identifier_token = consume(TokenType::IDENTIFIER, "Expected identifier");

    // Extract the identifier name from the token
    std::string name(identifier_token.value);

    // Create source location for error reporting
    SourceLocation loc(identifier_token.filename, identifier_token.line, identifier_token.column);

    // Create and return IdentifierExpr node
    // NOTE: We do NOT check if this variable exists - that's semantic analysis!
    return std::make_unique<IdentifierExpr>(name, loc);
}

// ============================================================================
// Literal Parsing (Support method)
// ============================================================================

std::unique_ptr<Expression> Parser::parseLiteral()
{
    Token literal_token = current_token_;
    advance();

    LiteralExpr::LiteralType lit_type;
    std::string value;

    switch (literal_token.type)
    {
    case TokenType::INT_LITERAL:
        lit_type = LiteralExpr::LiteralType::INTEGER;
        value = std::string(literal_token.value);
        break;

    case TokenType::FLOAT_LITERAL:
        lit_type = LiteralExpr::LiteralType::FLOAT;
        value = std::string(literal_token.value);
        break;

    case TokenType::STRING_LITERAL:
        lit_type = LiteralExpr::LiteralType::STRING;
        // Use processed_value for string literals (handles escape sequences)
        value = literal_token.processed_value;
        break;

    case TokenType::CHAR_LITERAL:
        lit_type = LiteralExpr::LiteralType::CHAR;
        // Use processed_value for char literals (handles escape sequences)
        value = literal_token.processed_value;
        break;

    default:
        reportError("Unknown literal type");
        throw std::runtime_error("Unknown literal type");
    }

    SourceLocation loc(literal_token.filename, literal_token.line, literal_token.column);
    return std::make_unique<LiteralExpr>(value, lit_type, loc);
}
