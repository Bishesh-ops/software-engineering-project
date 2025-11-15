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
    // Parse binary expressions with operator precedence
    return parseBinaryExpression(0);
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

// ============================================================================
// Binary Expression Parsing (USER STORY #3)
// ============================================================================

std::unique_ptr<Expression> Parser::parseBinaryExpression(int min_precedence)
{
    // Precedence Climbing Algorithm
    // Handles binary operators with correct precedence and left-to-right associativity

    // Start with a primary expression (left-hand side)
    auto left = parsePrimaryExpression();

    // Keep parsing binary operators while precedence allows
    while (isBinaryOperator(current_token_.type))
    {
        int current_precedence = getOperatorPrecedence(current_token_.type);

        // Stop if we've reached a lower precedence operator
        if (current_precedence < min_precedence)
        {
            break;
        }

        // Save operator token and advance
        Token op_token = current_token_;
        advance();

        // Parse right-hand side with higher precedence
        // For left-to-right associativity, use current_precedence + 1
        auto right = parseBinaryExpression(current_precedence + 1);

        // Create binary expression node
        std::string op_string = tokenTypeToOperatorString(op_token.type);
        SourceLocation loc(op_token.filename, op_token.line, op_token.column);

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op_string,
            std::move(right),
            loc
        );
    }

    return left;
}

// ============================================================================
// Operator Precedence Helpers (USER STORY #3)
// ============================================================================

int Parser::getOperatorPrecedence(TokenType type) const
{
    // Precedence levels as specified in User Story #3:
    // Higher number = higher precedence (binds tighter)

    switch (type)
    {
        // Level 1: Logical OR (lowest precedence)
        case TokenType::OP_OR:
            return 1;

        // Level 2: Logical AND
        case TokenType::OP_AND:
            return 2;

        // Level 3: Equality
        case TokenType::OP_EQ:
        case TokenType::OP_NE:
            return 3;

        // Level 4: Relational
        case TokenType::OP_LT:
        case TokenType::OP_GT:
        case TokenType::OP_LE:
        case TokenType::OP_GE:
            return 4;

        // Level 5: Additive
        case TokenType::OP_PLUS:
        case TokenType::OP_MINUS:
            return 5;

        // Level 6: Multiplicative (highest precedence)
        case TokenType::OP_STAR:
        case TokenType::OP_SLASH:
        case TokenType::OP_MOD:
            return 6;

        default:
            return 0; // Not a binary operator
    }
}

bool Parser::isBinaryOperator(TokenType type) const
{
    return getOperatorPrecedence(type) > 0;
}

std::string Parser::tokenTypeToOperatorString(TokenType type) const
{
    // Convert TokenType to operator string for AST
    switch (type)
    {
        // Arithmetic
        case TokenType::OP_PLUS:    return "+";
        case TokenType::OP_MINUS:   return "-";
        case TokenType::OP_STAR:    return "*";
        case TokenType::OP_SLASH:   return "/";
        case TokenType::OP_MOD:     return "%";

        // Comparison
        case TokenType::OP_EQ:      return "==";
        case TokenType::OP_NE:      return "!=";
        case TokenType::OP_LT:      return "<";
        case TokenType::OP_GT:      return ">";
        case TokenType::OP_LE:      return "<=";
        case TokenType::OP_GE:      return ">=";

        // Logical
        case TokenType::OP_AND:     return "&&";
        case TokenType::OP_OR:      return "||";

        default:
            return "?"; // Unknown operator
    }
}
