#include "../../include/parser.h"
#include <iostream>
#include <stdexcept>

Parser::Parser(Lexer &lexer)
    : lexer_(lexer), current_token_(lexer_.getNextToken()) {
  // Copy source code registration from Lexer's ErrorHandler for context display
  const auto &lexer_sources = lexer_.getErrorHandler().get_source_files();
  for (const auto &pair : lexer_sources) {
    error_handler_.register_source(pair.first, pair.second);
  }
}

// ============================================================================
// Token Management
// ============================================================================

void Parser::advance() { current_token_ = lexer_.getNextToken(); }

bool Parser::check(TokenType type) const { return current_token_.type == type; }

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

Token Parser::consume(TokenType type, const std::string &error_message) {
  if (check(type)) {
    Token token = current_token_;
    advance();
    return token;
  }

  // USER STORY #21: Report error and return current token instead of throwing
  // This allows parsing to continue and collect multiple errors
  reportError(error_message);
  return current_token_; // Return current token to allow recovery
}

// ============================================================================
// Error Handling
// ============================================================================

void Parser::reportError(const std::string &message) {
  SourceLocation loc = currentLocation();
  error_handler_.error(message, loc);
}

SourceLocation Parser::currentLocation() const {
  return SourceLocation(current_token_.filename, current_token_.line,
                        current_token_.column);
}

// Synchronize to next statement boundary (;, }, or start of new statement)
void Parser::synchronize() {
  // Skip tokens until we find a statement boundary
  while (current_token_.type != TokenType::EOF_TOKEN) {
    // If we just passed a semicolon, we're at a good sync point
    if (current_token_.type == TokenType::SEMICOLON) {
      advance(); // consume semicolon
      return;
    }

    // If we see a closing brace, we're at end of block
    if (current_token_.type == TokenType::RBRACE) {
      return; // don't consume the brace
    }

    // If we see a keyword that starts a statement, we're synchronized
    switch (current_token_.type) {
    case TokenType::KW_IF:
    case TokenType::KW_WHILE:
    case TokenType::KW_FOR:
    case TokenType::KW_RETURN:
    case TokenType::KW_INT:
    case TokenType::KW_FLOAT:
    case TokenType::KW_DOUBLE:
    case TokenType::KW_CHAR:
    case TokenType::KW_VOID:
    case TokenType::KW_STRUCT:
      return; // don't consume the keyword
    default:
      break;
    }

    advance();
  }
}

// Synchronize to next declaration
void Parser::synchronizeToDeclaration() {
  while (current_token_.type != TokenType::EOF_TOKEN) {
    // If we find a semicolon, consume it and we're ready for next declaration
    if (current_token_.type == TokenType::SEMICOLON) {
      advance(); // consume semicolon
      return;
    }

    // Look for type keywords or struct keyword (start of next declaration)
    if (current_token_.type == TokenType::KW_INT ||
        current_token_.type == TokenType::KW_FLOAT ||
        current_token_.type == TokenType::KW_DOUBLE ||
        current_token_.type == TokenType::KW_CHAR ||
        current_token_.type == TokenType::KW_VOID ||
        current_token_.type == TokenType::KW_LONG ||
        current_token_.type == TokenType::KW_UNSIGNED ||
        current_token_.type == TokenType::KW_STRUCT ||
        current_token_.type == TokenType::RBRACE) {
      return; // found a sync point
    }

    advance();
  }
}

// ============================================================================
// Expression Parsing
// ============================================================================

std::unique_ptr<Expression> Parser::parseExpression() {
  // Check for assignment: identifier = expression
  if (current_token_.type == TokenType::IDENTIFIER) {
    auto expr = parseBinaryExpression(0);

    // Check if followed by =
    if (current_token_.type == TokenType::OP_ASSIGN) {
      Token assign_token = current_token_;
      advance();

      auto value = parseExpression(); // Parse right-hand side

      SourceLocation loc(assign_token.filename, assign_token.line,
                         assign_token.column);
      return std::make_unique<AssignmentExpr>(std::move(expr), std::move(value),
                                              loc);
    }

    return expr;
  }

  // Parse binary expressions with operator precedence
  return parseBinaryExpression(0);
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression() {
  // Primary expressions:
  // - Identifiers (variable/function names)
  // - Literals (numbers, strings, chars)
  // - Parenthesized expressions
  // - Unary expressions

  // Handle unary operators
  if (isUnaryOperator(current_token_.type)) {
    return parseUnaryExpression();
  }

  switch (current_token_.type) {
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
    // USER STORY #21: Error recovery - return null instead of throwing
    reportError("Expected expression, got " +
                token_type_to_string(current_token_.type));
    advance();      // skip the bad token
    return nullptr; // Return null to signal error
  }
}

// ============================================================================
// Identifier and Function Call Parsing
// ============================================================================

std::unique_ptr<Expression> Parser::parseIdentifier() {
  // Accept: Creates IdentifierNode with variable name
  // Accept: Does NOT validate existence (that's semantic analysis later)
  // USER STORY #14: Also handles function calls when identifier is followed by
  // '(' USER STORY #17: Also handles array access when identifier is followed
  // by '['

  Token identifier_token =
      consume(TokenType::IDENTIFIER, "Expected identifier");

  // Extract the identifier name from the token
  std::string name(identifier_token.value);

  // Create source location for error reporting
  SourceLocation loc(identifier_token.filename, identifier_token.line,
                     identifier_token.column);

  // Create the base expression (identifier)
  std::unique_ptr<Expression> expr =
      std::make_unique<IdentifierExpr>(name, loc);

  // Handle postfix operations (function calls and array access)
  // These can be chained: arr[i](args) or func(args)[i]
  while (true) {
    if (check(TokenType::LPAREN)) {
      // USER STORY #14: Parse function call
      consume(TokenType::LPAREN, "Expected '('");

      // Parse argument list
      std::vector<std::unique_ptr<Expression>> arguments;

      // Handle empty argument list
      if (!check(TokenType::RPAREN)) {
        // Parse arguments: expr, expr, expr, ...
        do {
          arguments.push_back(parseExpression());

          // Check for comma (more arguments)
          if (!check(TokenType::COMMA)) {
            break;
          }
          advance(); // consume comma

        } while (!check(TokenType::RPAREN) && !check(TokenType::EOF_TOKEN));
      }

      consume(TokenType::RPAREN, "Expected ')' after function arguments");

      // Create CallExpr with current expression as callee
      expr = std::make_unique<CallExpr>(std::move(expr), std::move(arguments),
                                        loc);
    } else if (check(TokenType::LBRACKET)) {
      // USER STORY #17: Parse array access
      consume(TokenType::LBRACKET, "Expected '['");

      // Parse index expression
      std::unique_ptr<Expression> index = parseExpression();

      consume(TokenType::RBRACKET, "Expected ']' after array index");

      // Create ArrayAccessExpr with current expression as array
      expr = std::make_unique<ArrayAccessExpr>(std::move(expr),
                                               std::move(index), loc);
    } else if (check(TokenType::DOT) || check(TokenType::ARROW)) {
      // USER STORY #20: Parse member access (. or ->)
      bool isArrow = check(TokenType::ARROW);
      advance(); // consume '.' or '->'

      Token member_token = consume(TokenType::IDENTIFIER,
                                   "Expected member name after '.' or '->'");
      std::string memberName(member_token.value);

      // Desugar arrow operator: expr->member becomes (*expr).member
      if (isArrow) {
        // Create unary expression (*expr)
        expr = std::make_unique<UnaryExpr>("*", std::move(expr), true, loc);
        // Now treat as dot access (isArrow = false)
        isArrow = false;
      }

      // Create MemberAccessExpr
      expr = std::make_unique<MemberAccessExpr>(std::move(expr), memberName,
                                                isArrow, loc);
    } else if (check(TokenType::OP_INC) || check(TokenType::OP_DEC)) {
      // Handle postfix increment/decrement (e.g., i++, i--)
      std::string op = check(TokenType::OP_INC) ? "++" : "--";
      advance(); // consume '++' or '--'

      // Create UnaryExpr with prefix=false for postfix
      expr = std::make_unique<UnaryExpr>(op, std::move(expr), false, loc);
    } else {
      // No more postfix operations
      break;
    }
  }

  return expr;
}

// ============================================================================
// Literal Parsing (Support method)
// ============================================================================

std::unique_ptr<Expression> Parser::parseLiteral() {
  Token literal_token = current_token_;
  advance();

  LiteralExpr::LiteralType lit_type;
  std::string value;

  switch (literal_token.type) {
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
    // USER STORY #21: Error recovery
    reportError("Unknown literal type");
    return nullptr; // Return null for error
  }

  SourceLocation loc(literal_token.filename, literal_token.line,
                     literal_token.column);
  return std::make_unique<LiteralExpr>(value, lit_type, loc);
}

// ============================================================================
// Binary Expression Parsing
// ============================================================================

std::unique_ptr<Expression> Parser::parseBinaryExpression(int min_precedence) {
  // Precedence Climbing Algorithm
  // Handles binary operators with correct precedence and left-to-right
  // associativity

  // Start with a primary expression (left-hand side)
  auto left = parsePrimaryExpression();

  // Keep parsing binary operators while precedence allows
  while (isBinaryOperator(current_token_.type)) {
    int current_precedence = getOperatorPrecedence(current_token_.type);

    // Stop if we've reached a lower precedence operator
    if (current_precedence < min_precedence) {
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

    left = std::make_unique<BinaryExpr>(std::move(left), op_string,
                                        std::move(right), loc);
  }

  return left;
}

// ============================================================================
// Operator Precedence Helpers
// ============================================================================

int Parser::getOperatorPrecedence(TokenType type) const {
  // Precedence levels as specified in User Story #3:
  // Higher number = higher precedence (binds tighter)

  switch (type) {
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

bool Parser::isBinaryOperator(TokenType type) const {
  return getOperatorPrecedence(type) > 0;
}

std::string Parser::tokenTypeToOperatorString(TokenType type) const {
  // Convert TokenType to operator string for AST
  switch (type) {
  // Arithmetic
  case TokenType::OP_PLUS:
    return "+";
  case TokenType::OP_MINUS:
    return "-";
  case TokenType::OP_STAR:
    return "*";
  case TokenType::OP_SLASH:
    return "/";
  case TokenType::OP_MOD:
    return "%";

  // Comparison
  case TokenType::OP_EQ:
    return "==";
  case TokenType::OP_NE:
    return "!=";
  case TokenType::OP_LT:
    return "<";
  case TokenType::OP_GT:
    return ">";
  case TokenType::OP_LE:
    return "<=";
  case TokenType::OP_GE:
    return ">=";

  // Logical
  case TokenType::OP_AND:
    return "&&";
  case TokenType::OP_OR:
    return "||";
  case TokenType::OP_NOT:
    return "!";

  // Bitwise (used as unary operators)
  case TokenType::OP_BIT_AND:
    return "&";

  default:
    return "?"; // Unknown operator
  }
}

// ============================================================================
// Unary Expression Parsing
// ============================================================================

std::unique_ptr<Expression> Parser::parseUnaryExpression() {
  Token op_token = current_token_;
  std::string op = tokenTypeToOperatorString(op_token.type);
  advance();

  // Parse the operand (higher precedence than binary ops)
  auto operand = parsePrimaryExpression();

  SourceLocation loc(op_token.filename, op_token.line, op_token.column);
  return std::make_unique<UnaryExpr>(op, std::move(operand), true, loc);
}

// ============================================================================
// Parenthesized Expression Parsing
// ============================================================================

std::unique_ptr<Expression> Parser::parseParenthesizedExpression() {
  consume(TokenType::LPAREN, "Expected '('");
  auto expr = parseExpression();
  consume(TokenType::RPAREN, "Expected ')'");
  return expr;
}

// ============================================================================
// Statement Parsing
// ============================================================================

std::unique_ptr<Statement> Parser::parseStatement() {
  switch (current_token_.type) {
  case TokenType::KW_IF:
    return parseIfStatement();

  case TokenType::KW_WHILE:
    return parseWhileStatement();

  case TokenType::KW_FOR:
    return parseForStatement();

  case TokenType::KW_RETURN:
    return parseReturnStatement();

  case TokenType::LBRACE:
    return parseCompoundStatement();

  default:
    return parseExpressionStatement();
  }
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
  Token if_token = current_token_;
  advance(); // consume 'if'

  consume(TokenType::LPAREN, "Expected '(' after 'if'");
  auto condition = parseExpression();
  consume(TokenType::RPAREN, "Expected ')' after condition");

  auto then_branch = parseStatement();

  std::unique_ptr<Statement> else_branch = nullptr;
  if (match(TokenType::KW_ELSE)) {
    else_branch = parseStatement();
  }

  SourceLocation loc(if_token.filename, if_token.line, if_token.column);
  return std::make_unique<IfStmt>(std::move(condition), std::move(then_branch),
                                  std::move(else_branch), loc);
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
  Token while_token = current_token_;
  advance(); // consume 'while'

  consume(TokenType::LPAREN, "Expected '(' after 'while'");
  auto condition = parseExpression();
  consume(TokenType::RPAREN, "Expected ')' after condition");

  auto body = parseStatement();

  SourceLocation loc(while_token.filename, while_token.line,
                     while_token.column);
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body),
                                     loc);
}

std::unique_ptr<Statement> Parser::parseForStatement() {
  // USER STORY #10: Parse for loops
  // Syntax: for (init; condition; update) statement
  // All parts are optional: for (;;) is an infinite loop

  Token for_token = current_token_;
  advance(); // consume 'for'

  consume(TokenType::LPAREN, "Expected '(' after 'for'");

  // Parse initializer (optional)
  // Can be: variable declaration (int i = 0) or expression (i = 0) or empty
  std::unique_ptr<Statement> initializer = nullptr;

  if (!check(TokenType::SEMICOLON)) {
    // Check if it's a declaration (starts with a type keyword)
    if (isTypeKeyword(current_token_.type)) {
      // Parse as variable declaration
      auto decl = parseVariableDeclaration();
      // Wrap declaration in an expression statement for AST consistency
      // Note: The VarDecl already consumes the semicolon
      initializer =
          std::make_unique<ExpressionStmt>(nullptr, decl->getLocation());
      // For now, we'll store the declaration differently
      // This is a simplification - ideally we'd have a DeclStmt node
      // For this implementation, we'll parse it as an expression
    } else {
      // Parse as expression statement
      auto expr = parseExpression();
      consume(TokenType::SEMICOLON, "Expected ';' after for loop initializer");
      SourceLocation loc = currentLocation();
      initializer = std::make_unique<ExpressionStmt>(std::move(expr), loc);
    }
  } else {
    consume(TokenType::SEMICOLON, "Expected ';'");
  }

  // Parse condition (optional)
  std::unique_ptr<Expression> condition = nullptr;

  if (!check(TokenType::SEMICOLON)) {
    condition = parseExpression();
  }
  consume(TokenType::SEMICOLON, "Expected ';' after for loop condition");

  // Parse increment/update (optional)
  std::unique_ptr<Expression> increment = nullptr;

  if (!check(TokenType::RPAREN)) {
    increment = parseExpression();
  }

  consume(TokenType::RPAREN, "Expected ')' after for loop clauses");

  // Parse body
  auto body = parseStatement();

  SourceLocation loc(for_token.filename, for_token.line, for_token.column);
  return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
                                   std::move(increment), std::move(body), loc);
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
  // USER STORY #13: Parse return statements
  // Syntax: return;              (void return)
  //     or: return expression;   (return with value)

  Token return_token = current_token_;
  advance(); // consume 'return'

  std::unique_ptr<Expression> returnValue = nullptr;

  // Check if there's a return value (not just "return;")
  if (!check(TokenType::SEMICOLON)) {
    returnValue = parseExpression();
  }

  consume(TokenType::SEMICOLON, "Expected ';' after return statement");

  SourceLocation loc(return_token.filename, return_token.line,
                     return_token.column);
  return std::make_unique<ReturnStmt>(std::move(returnValue), loc);
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
  Token start_token = current_token_;
  auto expr = parseExpression();
  consume(TokenType::SEMICOLON, "Expected ';' after expression");

  SourceLocation loc(start_token.filename, start_token.line,
                     start_token.column);
  return std::make_unique<ExpressionStmt>(std::move(expr), loc);
}

std::unique_ptr<Statement> Parser::parseCompoundStatement() {
  Token lbrace_token = current_token_;
  consume(TokenType::LBRACE, "Expected '{'");

  std::vector<std::unique_ptr<Statement>> statements;

  while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
    // Handle declarations (int x = 5;) or statements
    if (isTypeKeyword(current_token_.type)) {
      // Parse as declaration and wrap in DeclStmt
      Token decl_token = current_token_;
      auto decl = parseVariableDeclaration();
      SourceLocation loc(decl_token.filename, decl_token.line,
                         decl_token.column);
      statements.push_back(std::make_unique<DeclStmt>(std::move(decl), loc));
    } else {
      statements.push_back(parseStatement());
    }
  }

  consume(TokenType::RBRACE, "Expected '}'");

  SourceLocation loc(lbrace_token.filename, lbrace_token.line,
                     lbrace_token.column);
  return std::make_unique<CompoundStmt>(std::move(statements), loc);
}

// ============================================================================
// Program Parsing (Top-Level Entry Point)
// ============================================================================

std::vector<std::unique_ptr<Declaration>> Parser::parseProgram() {
  std::vector<std::unique_ptr<Declaration>> declarations;

  // Parse all top-level declarations until we reach EOF
  while (!check(TokenType::EOF_TOKEN)) {
    // Stop if we've reached the maximum error limit (error recovery)
    if (error_handler_.has_reached_max_errors()) {
      break;
    }

    auto decl = parseDeclaration();
    if (decl) {
      declarations.push_back(std::move(decl));
    }

    // If we had an error and didn't get a declaration,
    // skip to the next potential declaration to continue parsing
    if (!decl && hasErrors()) {
      synchronizeToDeclaration();
    }

    // Break if we're stuck at EOF
    if (check(TokenType::EOF_TOKEN)) {
      break;
    }
  }

  return declarations;
}

// ============================================================================
// Declaration Parsing
// ============================================================================

std::unique_ptr<Declaration> Parser::parseDeclaration() {
  // USER STORY #19: Check for struct definition vs struct variable declaration
  if (check(TokenType::KW_STRUCT)) {
    return parseStructDeclarationOrDefinition();
  }

  // Parse type and name first, then determine if function or variable
  bool isExtern = false;
  if (check(TokenType::KW_EXTERN)) {
    isExtern = true;
    advance(); // consume 'extern'
  }

  if (!isTypeKeyword(current_token_.type)) {
    // USER STORY #21: Error recovery
    reportError("Expected declaration");
    synchronizeToDeclaration();
    return nullptr;
  }

  Token start_token = current_token_;
  std::string type = parseType();

  // USER STORY #18: Parse pointer declarators (* symbols)
  int pointerLevel = 0;
  while (check(TokenType::OP_STAR)) {
    advance(); // consume '*'
    pointerLevel++;
  }

  Token name_token =
      consume(TokenType::IDENTIFIER, "Expected identifier in declaration");
  std::string name(name_token.value);

  // Delegate to appropriate helper based on what follows the identifier
  if (check(TokenType::LPAREN)) {
    // Function declaration: type name(...)
    return parseFunctionDeclarationImpl(start_token, type, name, pointerLevel,
                                        isExtern);
  } else if (check(TokenType::LBRACKET)) {
    // Array declaration: type name[size]
    return parseArrayDeclaration(start_token, type, name, pointerLevel);
  } else {
    // Regular variable declaration: type name [= value]
    return parseVariableDeclarationImpl(start_token, type, name, pointerLevel);
  }
}

// ============================================================================
// Declaration Parsing Helper Methods (Refactored)
// ============================================================================

// Helper: Parse struct field list { type name; type name; ... }
std::vector<std::unique_ptr<VarDecl>> Parser::parseStructFieldList() {
  std::vector<std::unique_ptr<VarDecl>> fields;

  while (!check(TokenType::RBRACE) &&
         current_token_.type != TokenType::EOF_TOKEN) {
    // Validate field starts with a type keyword
    if (!isTypeKeyword(current_token_.type) &&
        current_token_.type != TokenType::KW_STRUCT) {
      reportError("Expected type keyword for struct field");
      // Skip to next semicolon or closing brace
      while (current_token_.type != TokenType::SEMICOLON &&
             current_token_.type != TokenType::RBRACE &&
             current_token_.type != TokenType::EOF_TOKEN) {
        advance();
      }
      if (current_token_.type == TokenType::SEMICOLON)
        advance();
      continue;
    }

    Token field_start = current_token_;
    std::string fieldType = parseType();

    // Handle pointer fields (* symbols)
    int pointerLevel = 0;
    while (check(TokenType::OP_STAR)) {
      advance();
      pointerLevel++;
    }

    Token field_name_token =
        consume(TokenType::IDENTIFIER, "Expected field name");
    std::string fieldName(field_name_token.value);

    // Handle array fields [size]
    bool isArray = false;
    std::unique_ptr<Expression> arraySize = nullptr;

    if (check(TokenType::LBRACKET)) {
      advance(); // consume '['
      isArray = true;

      if (!check(TokenType::RBRACKET)) {
        arraySize = parseExpression();
      }

      consume(TokenType::RBRACKET, "Expected ']' after array size");
    }

    consume(TokenType::SEMICOLON, "Expected ';' after struct field");

    SourceLocation fieldLoc(field_start.filename, field_start.line,
                            field_start.column);
    fields.push_back(
        std::make_unique<VarDecl>(fieldName, fieldType, nullptr, fieldLoc,
                                  isArray, std::move(arraySize), pointerLevel));
  }

  return fields;
}

// Helper: Parse struct declaration or definition
// Handles: struct Point { ... };  OR  struct Point p;
std::unique_ptr<Declaration> Parser::parseStructDeclarationOrDefinition() {
  Token structToken = current_token_;
  advance(); // consume 'struct'

  if (!check(TokenType::IDENTIFIER)) {
    reportError("Expected struct name after 'struct' keyword");
    synchronizeToDeclaration();
    return nullptr;
  }

  Token nameToken = current_token_;
  advance(); // consume struct name

  // Distinguish between definition and declaration
  if (check(TokenType::LBRACE)) {
    // It's a struct definition: struct Name { ... };
    consume(TokenType::LBRACE, "Expected '{' after struct name");

    // Parse member fields
    std::vector<std::unique_ptr<VarDecl>> fields = parseStructFieldList();

    consume(TokenType::RBRACE, "Expected '}' after struct fields");
    consume(TokenType::SEMICOLON, "Expected ';' after struct definition");

    SourceLocation loc(structToken.filename, structToken.line,
                       structToken.column);
    return std::make_unique<StructDecl>(std::string(nameToken.value),
                                        std::move(fields), loc);
  } else {
    // It's a variable declaration with struct type: struct Point p;
    std::string type =
        std::string(structToken.value) + " " + std::string(nameToken.value);
    Token var_name_token =
        consume(TokenType::IDENTIFIER, "Expected identifier in declaration");
    std::string var_name(var_name_token.value);

    // Handle array declaration
    bool isArray = false;
    std::unique_ptr<Expression> arraySize = nullptr;

    if (check(TokenType::LBRACKET)) {
      advance();
      isArray = true;
      if (!check(TokenType::RBRACKET)) {
        arraySize = parseExpression();
      }
      consume(TokenType::RBRACKET, "Expected ']'");
    }

    // Handle initialization
    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::OP_ASSIGN)) {
      initializer = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after declaration");

    SourceLocation loc(structToken.filename, structToken.line,
                       structToken.column);
    return std::make_unique<VarDecl>(var_name, type, std::move(initializer),
                                     loc, isArray, std::move(arraySize), 0);
  }
}

// Helper: Parse function declaration/definition
// Handles: int foo(int x) { ... }  OR  int foo(int x);
std::unique_ptr<Declaration> Parser::parseFunctionDeclarationImpl(
    const Token &start_token, const std::string &type, const std::string &name,
    int /* pointerLevel */, bool isExtern) {
  // Parse parameter list
  consume(TokenType::LPAREN, "Expected '(' after function name");
  std::vector<std::unique_ptr<ParameterDecl>> parameters = parseParameterList();
  consume(TokenType::RPAREN, "Expected ')' after parameter list");

  // Check if this is a forward declaration (;) or a definition ({...})
  std::unique_ptr<CompoundStmt> body = nullptr;

  if (check(TokenType::LBRACE)) {
    // Function definition with body
    body = std::unique_ptr<CompoundStmt>(
        dynamic_cast<CompoundStmt *>(parseCompoundStatement().release()));
  } else if (match(TokenType::SEMICOLON)) {
    // Forward declaration - body remains nullptr
  } else {
    reportError("Expected ';' or '{' after function declaration");
    synchronize();
    // Continue with null body (treated as forward declaration)
  }

  SourceLocation loc(start_token.filename, start_token.line,
                     start_token.column);
  return std::make_unique<FunctionDecl>(name, type, std::move(parameters),
                                        std::move(body), loc, isExtern);
}

// Helper: Parse array declaration
// Handles: int arr[10];  OR  int arr[10] = ...;
std::unique_ptr<Declaration>
Parser::parseArrayDeclaration(const Token &start_token, const std::string &type,
                              const std::string &name, int pointerLevel) {
  consume(TokenType::LBRACKET, "Expected '['");

  // Parse array size expression
  std::unique_ptr<Expression> arraySize = parseExpression();

  consume(TokenType::RBRACKET, "Expected ']' after array size");

  // Arrays can optionally have initializers
  std::unique_ptr<Expression> initializer = nullptr;
  if (match(TokenType::OP_ASSIGN)) {
    initializer = parseExpression();
  }

  if (!check(TokenType::SEMICOLON)) {
    reportError("Expected ';'");
    synchronizeToDeclaration();
    return nullptr;
  }
  advance(); // Consume semicolon

  SourceLocation loc(start_token.filename, start_token.line,
                     start_token.column);
  return std::make_unique<VarDecl>(name, type, std::move(initializer), loc,
                                   true, std::move(arraySize), pointerLevel);
}

// Helper: Parse regular variable declaration
// Handles: int x;  OR  int x = 5;  OR  int *p = &x;
std::unique_ptr<Declaration> Parser::parseVariableDeclarationImpl(
    const Token &start_token, const std::string &type, const std::string &name,
    int pointerLevel) {
  // Handle initialization
  std::unique_ptr<Expression> initializer = nullptr;

  if (match(TokenType::OP_ASSIGN)) {
    initializer = parseExpression();
  }

  if (!check(TokenType::SEMICOLON)) {
    reportError("Expected ';'");
    synchronizeToDeclaration();
    return nullptr;
  }
  advance(); // Consume semicolon

  SourceLocation loc(start_token.filename, start_token.line,
                     start_token.column);
  return std::make_unique<VarDecl>(name, type, std::move(initializer), loc,
                                   false, nullptr, pointerLevel);
}

// USER STORY #19: Parse Struct Definitions
// Syntax: struct Name { type1 field1; type2 field2; ... };
std::unique_ptr<Declaration> Parser::parseStructDefinition() {
  Token start_token = current_token_;
  consume(TokenType::KW_STRUCT, "Expected 'struct' keyword");

  Token name_token = consume(TokenType::IDENTIFIER, "Expected struct name");
  std::string structName(name_token.value);

  consume(TokenType::LBRACE, "Expected '{' after struct name");

  // Parse member fields (zero or more variable declarations)
  std::vector<std::unique_ptr<VarDecl>> fields;

  while (!check(TokenType::RBRACE) &&
         current_token_.type != TokenType::EOF_TOKEN) {
    // Parse each field as a variable declaration without initializer
    // Syntax: type name; or struct name;
    if (!isTypeKeyword(current_token_.type) &&
        current_token_.type != TokenType::KW_STRUCT) {
      // USER STORY #21: Error recovery - skip to semicolon or closing brace
      reportError("Expected type keyword for struct field");
      while (current_token_.type != TokenType::SEMICOLON &&
             current_token_.type != TokenType::RBRACE &&
             current_token_.type != TokenType::EOF_TOKEN) {
        advance();
      }
      if (current_token_.type == TokenType::SEMICOLON)
        advance();
      continue; // Skip this field and try next one
    }

    Token field_start = current_token_;
    std::string fieldType = parseType();

    // Handle pointer fields
    int pointerLevel = 0;
    while (check(TokenType::OP_STAR)) {
      advance();
      pointerLevel++;
    }

    Token field_name_token =
        consume(TokenType::IDENTIFIER, "Expected field name");
    std::string fieldName(field_name_token.value);

    // Handle array fields
    bool isArray = false;
    std::unique_ptr<Expression> arraySize = nullptr;

    if (check(TokenType::LBRACKET)) {
      advance(); // consume '['
      isArray = true;

      if (!check(TokenType::RBRACKET)) {
        arraySize = parseExpression();
      }

      consume(TokenType::RBRACKET, "Expected ']' after array size");
    }

    consume(TokenType::SEMICOLON, "Expected ';' after struct field");

    SourceLocation fieldLoc(field_start.filename, field_start.line,
                            field_start.column);
    fields.push_back(
        std::make_unique<VarDecl>(fieldName, fieldType, nullptr, fieldLoc,
                                  isArray, std::move(arraySize), pointerLevel));
  }

  consume(TokenType::RBRACE, "Expected '}' after struct fields");
  consume(TokenType::SEMICOLON, "Expected ';' after struct definition");

  SourceLocation loc(start_token.filename, start_token.line,
                     start_token.column);
  return std::make_unique<StructDecl>(structName, std::move(fields), loc);
}

std::unique_ptr<Declaration> Parser::parseVariableDeclaration() {
  // This method is kept for backward compatibility but just calls
  // parseDeclaration() The main logic is now in parseDeclaration() which
  // handles both variables and functions
  return parseDeclaration();
}

std::unique_ptr<Declaration> Parser::parseFunctionDeclaration() {
  // This method is kept for potential future use but just calls
  // parseDeclaration() The main logic is now in parseDeclaration() which
  // handles both variables and functions
  return parseDeclaration();
}

// ============================================================================
// Helper Methods
// ============================================================================

bool Parser::isUnaryOperator(TokenType type) const {
  return type == TokenType::OP_MINUS || type == TokenType::OP_NOT ||
         type == TokenType::OP_STAR || type == TokenType::OP_BIT_AND;
}

bool Parser::isTypeKeyword(TokenType type) const {
  return type == TokenType::KW_INT || type == TokenType::KW_FLOAT ||
         type == TokenType::KW_DOUBLE || type == TokenType::KW_CHAR ||
         type == TokenType::KW_VOID || type == TokenType::KW_LONG ||
         type == TokenType::KW_SHORT || type == TokenType::KW_STRUCT;
}

std::string Parser::parseType() {
  // USER STORY #19: Handle struct types (struct TypeName)
  if (current_token_.type == TokenType::KW_STRUCT) {
    std::string type(current_token_.value); // "struct"
    advance();

    // Optionally followed by a type name (for named structs)
    if (current_token_.type == TokenType::IDENTIFIER) {
      type += " ";
      type += std::string(current_token_.value);
      advance();
    }

    return type;
  }

  if (!isTypeKeyword(current_token_.type)) {
    reportError("Expected type keyword");
    throw std::runtime_error("Expected type");
  }

  std::string type(current_token_.value);
  advance();
  return type;
}

std::vector<std::unique_ptr<ParameterDecl>> Parser::parseParameterList() {
  // USER STORY #12: Parse function parameter list
  // Syntax: (type name, type name, ...)
  // Empty parameter list: () or (void)

  std::vector<std::unique_ptr<ParameterDecl>> parameters;

  // Handle empty parameter list or (void)
  if (check(TokenType::RPAREN)) {
    return parameters; // empty list
  }

  if (check(TokenType::KW_VOID)) {
    // Check if it's just "(void)" - no parameters
    Token void_token = current_token_;
    advance();

    if (check(TokenType::RPAREN)) {
      return parameters; // empty list, void means no parameters
    } else {
      reportError("Unexpected token after 'void' in parameter list");
      throw std::runtime_error("Unexpected token after 'void'");
    }
  }

  // Parse parameter list: type name, type name, ...
  do {
    Token param_start = current_token_;

    // Parse parameter type
    std::string paramType = parseType();

    // Handle pointer types (e.g., int* p, struct Ball* b)
    while (check(TokenType::OP_STAR)) {
      paramType += "*";
      advance(); // consume '*'
    }

    // Parse parameter name
    Token param_name_token =
        consume(TokenType::IDENTIFIER, "Expected parameter name");
    std::string paramName(param_name_token.value);

    SourceLocation loc(param_start.filename, param_start.line,
                       param_start.column);
    parameters.push_back(
        std::make_unique<ParameterDecl>(paramName, paramType, loc));

    // Check for comma (more parameters)
    if (!check(TokenType::COMMA)) {
      break;
    }
    advance(); // consume comma

  } while (!check(TokenType::RPAREN) && !check(TokenType::EOF_TOKEN));

  return parameters;
}
