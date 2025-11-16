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
    // Check for assignment: identifier = expression
    if (current_token_.type == TokenType::IDENTIFIER)
    {
        auto expr = parseBinaryExpression(0);

        // Check if followed by =
        if (current_token_.type == TokenType::OP_ASSIGN)
        {
            Token assign_token = current_token_;
            advance();

            auto value = parseExpression();  // Parse right-hand side

            SourceLocation loc(assign_token.filename, assign_token.line, assign_token.column);
            return std::make_unique<AssignmentExpr>(std::move(expr), std::move(value), loc);
        }

        return expr;
    }

    // Parse binary expressions with operator precedence
    return parseBinaryExpression(0);
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression()
{
    // Primary expressions:
    // - Identifiers (variable/function names)
    // - Literals (numbers, strings, chars)
    // - Parenthesized expressions
    // - Unary expressions

    // Handle unary operators
    if (isUnaryOperator(current_token_.type))
    {
        return parseUnaryExpression();
    }

    switch (current_token_.type)
    {
    case TokenType::IDENTIFIER:
        return parseIdentifier();

    case TokenType::INT_LITERAL:
    case TokenType::FLOAT_LITERAL:
    case TokenType::STRING_LITERAL:
    case TokenType::CHAR_LITERAL:
        return parseLiteral();

    case TokenType::LPAREN:
        return parseParenthesizedExpression();

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
        case TokenType::OP_NOT:     return "!";

        // Bitwise (used as unary operators)
        case TokenType::OP_BIT_AND: return "&";

        default:
            return "?"; // Unknown operator
    }
}

// ============================================================================
// Unary Expression Parsing (USER STORY #4)
// ============================================================================

std::unique_ptr<Expression> Parser::parseUnaryExpression()
{
    Token op_token = current_token_;
    std::string op = tokenTypeToOperatorString(op_token.type);
    advance();

    // Parse the operand (higher precedence than binary ops)
    auto operand = parsePrimaryExpression();

    SourceLocation loc(op_token.filename, op_token.line, op_token.column);
    return std::make_unique<UnaryExpr>(op, std::move(operand), true, loc);
}

// ============================================================================
// Parenthesized Expression Parsing (USER STORY #5)
// ============================================================================

std::unique_ptr<Expression> Parser::parseParenthesizedExpression()
{
    consume(TokenType::LPAREN, "Expected '('");
    auto expr = parseExpression();
    consume(TokenType::RPAREN, "Expected ')'");
    return expr;
}

// ============================================================================
// Statement Parsing
// ============================================================================

std::unique_ptr<Statement> Parser::parseStatement()
{
    switch (current_token_.type)
    {
    case TokenType::KW_IF:
        return parseIfStatement();

    case TokenType::KW_WHILE:
        return parseWhileStatement();

    case TokenType::KW_FOR:
        return parseForStatement();

    case TokenType::LBRACE:
        return parseCompoundStatement();

    default:
        return parseExpressionStatement();
    }
}

std::unique_ptr<Statement> Parser::parseIfStatement()
{
    Token if_token = current_token_;
    advance();  // consume 'if'

    consume(TokenType::LPAREN, "Expected '(' after 'if'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto then_branch = parseStatement();

    std::unique_ptr<Statement> else_branch = nullptr;
    if (match(TokenType::KW_ELSE))
    {
        else_branch = parseStatement();
    }

    SourceLocation loc(if_token.filename, if_token.line, if_token.column);
    return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch), std::move(else_branch), loc);
}

std::unique_ptr<Statement> Parser::parseWhileStatement()
{
    Token while_token = current_token_;
    advance();  // consume 'while'

    consume(TokenType::LPAREN, "Expected '(' after 'while'");
    auto condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after condition");

    auto body = parseStatement();

    SourceLocation loc(while_token.filename, while_token.line, while_token.column);
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body), loc);
}

std::unique_ptr<Statement> Parser::parseForStatement()
{
    // USER STORY #10: Parse for loops
    // Syntax: for (init; condition; update) statement
    // All parts are optional: for (;;) is an infinite loop

    Token for_token = current_token_;
    advance();  // consume 'for'

    consume(TokenType::LPAREN, "Expected '(' after 'for'");

    // Parse initializer (optional)
    // Can be: variable declaration (int i = 0) or expression (i = 0) or empty
    std::unique_ptr<Statement> initializer = nullptr;

    if (!check(TokenType::SEMICOLON))
    {
        // Check if it's a declaration (starts with a type keyword)
        if (isTypeKeyword(current_token_.type))
        {
            // Parse as variable declaration
            auto decl = parseVariableDeclaration();
            // Wrap declaration in an expression statement for AST consistency
            // Note: The VarDecl already consumes the semicolon
            initializer = std::make_unique<ExpressionStmt>(nullptr, decl->getLocation());
            // For now, we'll store the declaration differently
            // This is a simplification - ideally we'd have a DeclStmt node
            // For this implementation, we'll parse it as an expression
        }
        else
        {
            // Parse as expression statement
            auto expr = parseExpression();
            consume(TokenType::SEMICOLON, "Expected ';' after for loop initializer");
            SourceLocation loc = currentLocation();
            initializer = std::make_unique<ExpressionStmt>(std::move(expr), loc);
        }
    }
    else
    {
        consume(TokenType::SEMICOLON, "Expected ';'");
    }

    // Parse condition (optional)
    std::unique_ptr<Expression> condition = nullptr;

    if (!check(TokenType::SEMICOLON))
    {
        condition = parseExpression();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after for loop condition");

    // Parse increment/update (optional)
    std::unique_ptr<Expression> increment = nullptr;

    if (!check(TokenType::RPAREN))
    {
        increment = parseExpression();
    }

    consume(TokenType::RPAREN, "Expected ')' after for loop clauses");

    // Parse body
    auto body = parseStatement();

    SourceLocation loc(for_token.filename, for_token.line, for_token.column);
    return std::make_unique<ForStmt>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body),
        loc
    );
}

std::unique_ptr<Statement> Parser::parseExpressionStatement()
{
    Token start_token = current_token_;
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");

    SourceLocation loc(start_token.filename, start_token.line, start_token.column);
    return std::make_unique<ExpressionStmt>(std::move(expr), loc);
}

std::unique_ptr<Statement> Parser::parseCompoundStatement()
{
    Token lbrace_token = current_token_;
    consume(TokenType::LBRACE, "Expected '{'");

    std::vector<std::unique_ptr<Statement>> statements;

    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN))
    {
        // Handle declarations (int x = 5;) or statements
        if (isTypeKeyword(current_token_.type))
        {
            // Parse as declaration, wrap in expression statement
            auto decl = parseVariableDeclaration();
            // For now, we can't directly add declarations to statement list
            // This is a simplification - real compilers handle this differently
        }
        else
        {
            statements.push_back(parseStatement());
        }
    }

    consume(TokenType::RBRACE, "Expected '}'");

    SourceLocation loc(lbrace_token.filename, lbrace_token.line, lbrace_token.column);
    return std::make_unique<CompoundStmt>(std::move(statements), loc);
}

// ============================================================================
// Declaration Parsing (USER STORY #6)
// ============================================================================

std::unique_ptr<Declaration> Parser::parseDeclaration()
{
    if (isTypeKeyword(current_token_.type))
    {
        return parseVariableDeclaration();
    }

    reportError("Expected declaration");
    throw std::runtime_error("Expected declaration");
}

std::unique_ptr<Declaration> Parser::parseVariableDeclaration()
{
    Token start_token = current_token_;
    std::string type = parseType();

    Token name_token = consume(TokenType::IDENTIFIER, "Expected variable name");
    std::string name(name_token.value);

    std::unique_ptr<Expression> initializer = nullptr;

    if (match(TokenType::OP_ASSIGN))
    {
        initializer = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    SourceLocation loc(start_token.filename, start_token.line, start_token.column);
    return std::make_unique<VarDecl>(name, type, std::move(initializer), loc);
}

// ============================================================================
// Helper Methods
// ============================================================================

bool Parser::isUnaryOperator(TokenType type) const
{
    return type == TokenType::OP_MINUS ||
           type == TokenType::OP_NOT ||
           type == TokenType::OP_STAR ||
           type == TokenType::OP_BIT_AND;
}

bool Parser::isTypeKeyword(TokenType type) const
{
    return type == TokenType::KW_INT ||
           type == TokenType::KW_FLOAT ||
           type == TokenType::KW_DOUBLE ||
           type == TokenType::KW_CHAR ||
           type == TokenType::KW_VOID ||
           type == TokenType::KW_LONG ||
           type == TokenType::KW_SHORT;
}

std::string Parser::parseType()
{
    if (!isTypeKeyword(current_token_.type))
    {
        reportError("Expected type keyword");
        throw std::runtime_error("Expected type");
    }

    std::string type(current_token_.value);
    advance();
    return type;
}
