#include "include/parser.h"
#include "include/lexer.h"
#include "include/ast.h"
#include <iostream>
#include <string>

using namespace std;

// ============================================================================
// Helper Functions
// ============================================================================

unique_ptr<Expression> parseExpr(const string& source)
{
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    return parser.parseExpression();
}

unique_ptr<Statement> parseStmt(const string& source)
{
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    return parser.parseStatement();
}

unique_ptr<Declaration> parseDecl(const string& source)
{
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    return parser.parseDeclaration();
}

bool isIdentifier(Expression* expr, const string& name)
{
    if (!expr || expr->getNodeType() != ASTNodeType::IDENTIFIER_EXPR) return false;
    IdentifierExpr* id = dynamic_cast<IdentifierExpr*>(expr);
    return id && id->getName() == name;
}

bool isLiteral(Expression* expr, const string& value)
{
    if (!expr || expr->getNodeType() != ASTNodeType::LITERAL_EXPR) return false;
    LiteralExpr* lit = dynamic_cast<LiteralExpr*>(expr);
    return lit && lit->getValue() == value;
}

bool isBinaryOp(Expression* expr, const string& op)
{
    if (!expr || expr->getNodeType() != ASTNodeType::BINARY_EXPR) return false;
    BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr);
    return bin && bin->getOperator() == op;
}

bool isUnaryOp(Expression* expr, const string& op)
{
    if (!expr || expr->getNodeType() != ASTNodeType::UNARY_EXPR) return false;
    UnaryExpr* unary = dynamic_cast<UnaryExpr*>(expr);
    return unary && unary->getOperator() == op;
}

bool isAssignment(Expression* expr)
{
    return expr && expr->getNodeType() == ASTNodeType::ASSIGNMENT_EXPR;
}

bool isCallExpr(Expression* expr)
{
    return expr && expr->getNodeType() == ASTNodeType::CALL_EXPR;
}

bool isArrayAccess(Expression* expr)
{
    return expr && expr->getNodeType() == ASTNodeType::ARRAY_ACCESS_EXPR;
}

Expression* left(Expression* expr)
{
    BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr);
    return bin ? bin->getLeft() : nullptr;
}

Expression* right(Expression* expr)
{
    BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr);
    return bin ? bin->getRight() : nullptr;
}

Expression* operand(Expression* expr)
{
    UnaryExpr* unary = dynamic_cast<UnaryExpr*>(expr);
    return unary ? unary->getOperand() : nullptr;
}

void pass(const string& msg) { cout << "  [PASS] " << msg << endl; }
void fail(const string& msg) { cout << "  [FAIL] " << msg << endl; }

// ============================================================================
// Tests
// ============================================================================

void test_identifiers()
{
    cout << "\n[TEST] Identifiers\n";

    auto expr1 = parseExpr("myVariable");
    if (isIdentifier(expr1.get(), "myVariable"))
        pass("Simple identifier");
    else
        fail("Simple identifier");

    auto expr2 = parseExpr("_private123");
    if (isIdentifier(expr2.get(), "_private123"))
        pass("Identifier with underscore and numbers");
    else
        fail("Identifier with underscore and numbers");
}

void test_literals()
{
    cout << "\n[TEST] Literals\n";

    auto expr1 = parseExpr("42");
    if (isLiteral(expr1.get(), "42"))
        pass("Integer literal");
    else
        fail("Integer literal");

    auto expr2 = parseExpr("3.14");
    if (isLiteral(expr2.get(), "3.14"))
        pass("Float literal");
    else
        fail("Float literal");
}

void test_binary_ops()
{
    cout << "\n[TEST] Binary Operations\n";

    auto expr = parseExpr("a + b");
    if (isBinaryOp(expr.get(), "+") &&
        isIdentifier(left(expr.get()), "a") &&
        isIdentifier(right(expr.get()), "b"))
        pass("a + b");
    else
        fail("a + b");

    auto expr2 = parseExpr("x * y");
    if (isBinaryOp(expr2.get(), "*") &&
        isIdentifier(left(expr2.get()), "x") &&
        isIdentifier(right(expr2.get()), "y"))
        pass("x * y");
    else
        fail("x * y");
}

void test_unary_ops()
{
    cout << "\n[TEST] Unary Operations\n";

    auto expr1 = parseExpr("-x");
    if (isUnaryOp(expr1.get(), "-") && isIdentifier(operand(expr1.get()), "x"))
        pass("Negation: -x");
    else
        fail("Negation: -x");

    auto expr2 = parseExpr("!flag");
    if (isUnaryOp(expr2.get(), "!") && isIdentifier(operand(expr2.get()), "flag"))
        pass("Logical not: !flag");
    else
        fail("Logical not: !flag");

    auto expr3 = parseExpr("*ptr");
    if (isUnaryOp(expr3.get(), "*") && isIdentifier(operand(expr3.get()), "ptr"))
        pass("Dereference: *ptr");
    else
        fail("Dereference: *ptr");

    auto expr4 = parseExpr("&var");
    if (isUnaryOp(expr4.get(), "&") && isIdentifier(operand(expr4.get()), "var"))
        pass("Address-of: &var");
    else
        fail("Address-of: &var");
}

void test_parentheses()
{
    cout << "\n[TEST] Parenthesized Expressions\n";

    auto expr1 = parseExpr("(5)");
    if (isLiteral(expr1.get(), "5"))
        pass("Simple: (5)");
    else
        fail("Simple: (5)");

    auto expr2 = parseExpr("(a + b) * c");
    if (isBinaryOp(expr2.get(), "*") &&
        isBinaryOp(left(expr2.get()), "+") &&
        isIdentifier(right(expr2.get()), "c"))
        pass("Precedence override: (a + b) * c");
    else
        fail("Precedence override: (a + b) * c");
}

void test_assignments()
{
    cout << "\n[TEST] Assignment Expressions\n";

    auto expr1 = parseExpr("x = 5");
    if (isAssignment(expr1.get()))
    {
        AssignmentExpr* assign = dynamic_cast<AssignmentExpr*>(expr1.get());
        if (isIdentifier(assign->getTarget(), "x") &&
            isLiteral(assign->getValue(), "5"))
            pass("Simple assignment: x = 5");
        else
            fail("Simple assignment: x = 5");
    }
    else
        fail("Simple assignment: x = 5");

    auto expr2 = parseExpr("y = a + b");
    if (isAssignment(expr2.get()))
    {
        AssignmentExpr* assign = dynamic_cast<AssignmentExpr*>(expr2.get());
        if (isIdentifier(assign->getTarget(), "y") &&
            isBinaryOp(assign->getValue(), "+"))
            pass("Assignment with expression: y = a + b");
        else
            fail("Assignment with expression: y = a + b");
    }
    else
        fail("Assignment with expression: y = a + b");
}

void test_variable_declarations()
{
    cout << "\n[TEST] Variable Declarations\n";

    auto decl1 = parseDecl("int x;");
    if (decl1 && decl1->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl1.get());
        if (var && var->getName() == "x" && var->getType() == "int" && !var->getInitializer())
            pass("Simple declaration: int x;");
        else
            fail("Simple declaration: int x;");
    }
    else
        fail("Simple declaration: int x;");

    auto decl2 = parseDecl("int y = 42;");
    if (decl2 && decl2->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl2.get());
        if (var && var->getName() == "y" && var->getType() == "int" &&
            isLiteral(var->getInitializer(), "42"))
            pass("With initialization: int y = 42;");
        else
            fail("With initialization: int y = 42;");
    }
    else
        fail("With initialization: int y = 42;");
}

void test_if_statements()
{
    cout << "\n[TEST] If Statements\n";

    auto stmt1 = parseStmt("if (x > 0) y = 1;");
    if (stmt1 && stmt1->getNodeType() == ASTNodeType::IF_STMT)
    {
        IfStmt* if_stmt = dynamic_cast<IfStmt*>(stmt1.get());
        if (if_stmt && isBinaryOp(if_stmt->getCondition(), ">") &&
            if_stmt->getThenBranch() && !if_stmt->getElseBranch())
            pass("Simple if: if (x > 0) y = 1;");
        else
            fail("Simple if: if (x > 0) y = 1;");
    }
    else
        fail("Simple if: if (x > 0) y = 1;");

    auto stmt2 = parseStmt("if (x > 0) y = 1; else y = -1;");
    if (stmt2 && stmt2->getNodeType() == ASTNodeType::IF_STMT)
    {
        IfStmt* if_stmt = dynamic_cast<IfStmt*>(stmt2.get());
        if (if_stmt && if_stmt->getThenBranch() && if_stmt->getElseBranch())
            pass("If-else: if (x > 0) y = 1; else y = -1;");
        else
            fail("If-else: if (x > 0) y = 1; else y = -1;");
    }
    else
        fail("If-else: if (x > 0) y = 1; else y = -1;");
}

void test_while_loops()
{
    cout << "\n[TEST] While Loops\n";

    auto stmt = parseStmt("while (x < 10) x = x + 1;");
    if (stmt && stmt->getNodeType() == ASTNodeType::WHILE_STMT)
    {
        WhileStmt* while_stmt = dynamic_cast<WhileStmt*>(stmt.get());
        if (while_stmt && isBinaryOp(while_stmt->getCondition(), "<") &&
            while_stmt->getBody())
            pass("While loop: while (x < 10) x = x + 1;");
        else
            fail("While loop: while (x < 10) x = x + 1;");
    }
    else
        fail("While loop: while (x < 10) x = x + 1;");
}

void test_for_loops()
{
    cout << "\n[TEST] For Loops (USER STORY #10)\n";

    // Test 1: Full for loop with all parts
    auto stmt1 = parseStmt("for (i = 0; i < 10; i = i + 1) x = x + 1;");
    if (stmt1 && stmt1->getNodeType() == ASTNodeType::FOR_STMT)
    {
        ForStmt* for_stmt = dynamic_cast<ForStmt*>(stmt1.get());
        if (for_stmt &&
            for_stmt->getInitializer() &&
            for_stmt->getCondition() &&
            isBinaryOp(for_stmt->getCondition(), "<") &&
            for_stmt->getIncrement() &&
            for_stmt->getBody())
            pass("Full for loop: for (i = 0; i < 10; i = i + 1) x = x + 1;");
        else
            fail("Full for loop: for (i = 0; i < 10; i = i + 1) x = x + 1;");
    }
    else
        fail("Full for loop: for (i = 0; i < 10; i = i + 1) x = x + 1;");

    // Test 2: Infinite loop - for (;;)
    auto stmt2 = parseStmt("for (;;) x = x + 1;");
    if (stmt2 && stmt2->getNodeType() == ASTNodeType::FOR_STMT)
    {
        ForStmt* for_stmt = dynamic_cast<ForStmt*>(stmt2.get());
        if (for_stmt &&
            !for_stmt->getInitializer() &&
            !for_stmt->getCondition() &&
            !for_stmt->getIncrement() &&
            for_stmt->getBody())
            pass("Infinite loop: for (;;) x = x + 1;");
        else
            fail("Infinite loop: for (;;) x = x + 1;");
    }
    else
        fail("Infinite loop: for (;;) x = x + 1;");

    // Test 3: For loop with only condition
    auto stmt3 = parseStmt("for (; i < 10;) x = x + 1;");
    if (stmt3 && stmt3->getNodeType() == ASTNodeType::FOR_STMT)
    {
        ForStmt* for_stmt = dynamic_cast<ForStmt*>(stmt3.get());
        if (for_stmt &&
            !for_stmt->getInitializer() &&
            for_stmt->getCondition() &&
            isBinaryOp(for_stmt->getCondition(), "<") &&
            !for_stmt->getIncrement() &&
            for_stmt->getBody())
            pass("For with only condition: for (; i < 10;) x = x + 1;");
        else
            fail("For with only condition: for (; i < 10;) x = x + 1;");
    }
    else
        fail("For with only condition: for (; i < 10;) x = x + 1;");

    // Test 4: For loop with init and condition only
    auto stmt4 = parseStmt("for (i = 0; i < 10;) x = x + 1;");
    if (stmt4 && stmt4->getNodeType() == ASTNodeType::FOR_STMT)
    {
        ForStmt* for_stmt = dynamic_cast<ForStmt*>(stmt4.get());
        if (for_stmt &&
            for_stmt->getInitializer() &&
            for_stmt->getCondition() &&
            !for_stmt->getIncrement() &&
            for_stmt->getBody())
            pass("For with init and condition: for (i = 0; i < 10;) x = x + 1;");
        else
            fail("For with init and condition: for (i = 0; i < 10;) x = x + 1;");
    }
    else
        fail("For with init and condition: for (i = 0; i < 10;) x = x + 1;");
}

void test_compound_statements()
{
    cout << "\n[TEST] Compound Statements (USER STORY #11)\n";

    // Test 1: Block with 2 statements
    auto stmt1 = parseStmt("{ x = 1; y = 2; }");
    if (stmt1 && stmt1->getNodeType() == ASTNodeType::COMPOUND_STMT)
    {
        CompoundStmt* compound = dynamic_cast<CompoundStmt*>(stmt1.get());
        if (compound && compound->getStatements().size() == 2)
            pass("Block with 2 statements: { x = 1; y = 2; }");
        else
            fail("Block with 2 statements: { x = 1; y = 2; }");
    }
    else
        fail("Block with 2 statements: { x = 1; y = 2; }");

    // Test 2: Empty block (Acceptance Criteria)
    auto stmt2 = parseStmt("{ }");
    if (stmt2 && stmt2->getNodeType() == ASTNodeType::COMPOUND_STMT)
    {
        CompoundStmt* compound = dynamic_cast<CompoundStmt*>(stmt2.get());
        if (compound && compound->getStatements().size() == 0)
            pass("Empty block: { }");
        else
            fail("Empty block: { }");
    }
    else
        fail("Empty block: { }");

    // Test 3: Nested blocks (Acceptance Criteria)
    auto stmt3 = parseStmt("{ x = 1; { y = 2; } }");
    if (stmt3 && stmt3->getNodeType() == ASTNodeType::COMPOUND_STMT)
    {
        CompoundStmt* outer = dynamic_cast<CompoundStmt*>(stmt3.get());
        if (outer && outer->getStatements().size() == 2)
        {
            Statement* inner_stmt = outer->getStatements()[1].get();
            if (inner_stmt && inner_stmt->getNodeType() == ASTNodeType::COMPOUND_STMT)
            {
                CompoundStmt* inner = dynamic_cast<CompoundStmt*>(inner_stmt);
                if (inner && inner->getStatements().size() == 1)
                    pass("Nested blocks: { x = 1; { y = 2; } }");
                else
                    fail("Nested blocks: { x = 1; { y = 2; } }");
            }
            else
                fail("Nested blocks: { x = 1; { y = 2; } }");
        }
        else
            fail("Nested blocks: { x = 1; { y = 2; } }");
    }
    else
        fail("Nested blocks: { x = 1; { y = 2; } }");

    // Test 4: Block with multiple statements
    auto stmt4 = parseStmt("{ a = 1; b = 2; c = 3; d = 4; }");
    if (stmt4 && stmt4->getNodeType() == ASTNodeType::COMPOUND_STMT)
    {
        CompoundStmt* compound = dynamic_cast<CompoundStmt*>(stmt4.get());
        if (compound && compound->getStatements().size() == 4)
            pass("Block with 4 statements");
        else
            fail("Block with 4 statements");
    }
    else
        fail("Block with 4 statements");

    // Test 5: Deeply nested blocks
    auto stmt5 = parseStmt("{ { { x = 1; } } }");
    if (stmt5 && stmt5->getNodeType() == ASTNodeType::COMPOUND_STMT)
    {
        CompoundStmt* level1 = dynamic_cast<CompoundStmt*>(stmt5.get());
        if (level1 && level1->getStatements().size() == 1)
        {
            Statement* level2_stmt = level1->getStatements()[0].get();
            if (level2_stmt && level2_stmt->getNodeType() == ASTNodeType::COMPOUND_STMT)
            {
                CompoundStmt* level2 = dynamic_cast<CompoundStmt*>(level2_stmt);
                if (level2 && level2->getStatements().size() == 1)
                {
                    Statement* level3_stmt = level2->getStatements()[0].get();
                    if (level3_stmt && level3_stmt->getNodeType() == ASTNodeType::COMPOUND_STMT)
                        pass("Triple nested blocks: { { { x = 1; } } }");
                    else
                        fail("Triple nested blocks: { { { x = 1; } } }");
                }
                else
                    fail("Triple nested blocks: { { { x = 1; } } }");
            }
            else
                fail("Triple nested blocks: { { { x = 1; } } }");
        }
        else
            fail("Triple nested blocks: { { { x = 1; } } }");
    }
    else
        fail("Triple nested blocks: { { { x = 1; } } }");

    // Test 6: Block in if statement
    auto stmt6 = parseStmt("if (x > 0) { y = 1; z = 2; }");
    if (stmt6 && stmt6->getNodeType() == ASTNodeType::IF_STMT)
    {
        IfStmt* if_stmt = dynamic_cast<IfStmt*>(stmt6.get());
        if (if_stmt && if_stmt->getThenBranch())
        {
            Statement* then_branch = if_stmt->getThenBranch();
            if (then_branch->getNodeType() == ASTNodeType::COMPOUND_STMT)
            {
                CompoundStmt* block = dynamic_cast<CompoundStmt*>(then_branch);
                if (block && block->getStatements().size() == 2)
                    pass("Block in if statement");
                else
                    fail("Block in if statement");
            }
            else
                fail("Block in if statement");
        }
        else
            fail("Block in if statement");
    }
    else
        fail("Block in if statement");

    // Test 7: Block in while loop
    auto stmt7 = parseStmt("while (i < 10) { x = x + 1; i = i + 1; }");
    if (stmt7 && stmt7->getNodeType() == ASTNodeType::WHILE_STMT)
    {
        WhileStmt* while_stmt = dynamic_cast<WhileStmt*>(stmt7.get());
        if (while_stmt && while_stmt->getBody())
        {
            Statement* body = while_stmt->getBody();
            if (body->getNodeType() == ASTNodeType::COMPOUND_STMT)
            {
                CompoundStmt* block = dynamic_cast<CompoundStmt*>(body);
                if (block && block->getStatements().size() == 2)
                    pass("Block in while loop");
                else
                    fail("Block in while loop");
            }
            else
                fail("Block in while loop");
        }
        else
            fail("Block in while loop");
    }
    else
        fail("Block in while loop");
}

void test_function_declarations()
{
    cout << "\n[TEST] Function Declarations (USER STORY #12)\n";

    // Test 1: Simple function declaration - int add(int a, int b);
    auto decl1 = parseDecl("int add(int a, int b);");
    if (decl1 && decl1->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl1.get());
        if (func && func->getName() == "add" &&
            func->getReturnType() == "int" &&
            func->getParameters().size() == 2 &&
            !func->getBody())
        {
            // Check parameter 1
            const auto& params = func->getParameters();
            if (params[0]->getName() == "a" && params[0]->getType() == "int" &&
                params[1]->getName() == "b" && params[1]->getType() == "int")
                pass("Forward declaration: int add(int a, int b);");
            else
                fail("Forward declaration: int add(int a, int b); - parameter mismatch");
        }
        else
            fail("Forward declaration: int add(int a, int b);");
    }
    else
        fail("Forward declaration: int add(int a, int b);");

    // Test 2: void function with no parameters - void printHello();
    auto decl2 = parseDecl("void printHello();");
    if (decl2 && decl2->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl2.get());
        if (func && func->getName() == "printHello" &&
            func->getReturnType() == "void" &&
            func->getParameters().size() == 0 &&
            !func->getBody())
            pass("Void function: void printHello();");
        else
            fail("Void function: void printHello();");
    }
    else
        fail("Void function: void printHello();");

    // Test 3: Function with void parameter - int getValue(void);
    auto decl3 = parseDecl("int getValue(void);");
    if (decl3 && decl3->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl3.get());
        if (func && func->getName() == "getValue" &&
            func->getReturnType() == "int" &&
            func->getParameters().size() == 0 &&
            !func->getBody())
            pass("Function with void param: int getValue(void);");
        else
            fail("Function with void param: int getValue(void);");
    }
    else
        fail("Function with void param: int getValue(void);");

    // Test 4: Function with multiple parameters
    auto decl4 = parseDecl("float calculate(int x, float y, char op);");
    if (decl4 && decl4->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl4.get());
        if (func && func->getName() == "calculate" &&
            func->getReturnType() == "float" &&
            func->getParameters().size() == 3 &&
            !func->getBody())
        {
            const auto& params = func->getParameters();
            if (params[0]->getName() == "x" && params[0]->getType() == "int" &&
                params[1]->getName() == "y" && params[1]->getType() == "float" &&
                params[2]->getName() == "op" && params[2]->getType() == "char")
                pass("Multiple parameters: float calculate(int x, float y, char op);");
            else
                fail("Multiple parameters - parameter mismatch");
        }
        else
            fail("Multiple parameters: float calculate(int x, float y, char op);");
    }
    else
        fail("Multiple parameters: float calculate(int x, float y, char op);");

    // Test 5: Function definition with body
    auto decl5 = parseDecl("int square(int n) { x = n * n; }");
    if (decl5 && decl5->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl5.get());
        if (func && func->getName() == "square" &&
            func->getReturnType() == "int" &&
            func->getParameters().size() == 1 &&
            func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 1)
                pass("Function definition: int square(int n) { x = n * n; }");
            else
                fail("Function definition: int square(int n) { x = n * n; }");
        }
        else
            fail("Function definition: int square(int n) { x = n * n; }");
    }
    else
        fail("Function definition: int square(int n) { x = n * n; }");

    // Test 6: Empty function body
    auto decl6 = parseDecl("void doNothing() { }");
    if (decl6 && decl6->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl6.get());
        if (func && func->getName() == "doNothing" &&
            func->getReturnType() == "void" &&
            func->getParameters().size() == 0 &&
            func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 0)
                pass("Empty function body: void doNothing() { }");
            else
                fail("Empty function body: void doNothing() { }");
        }
        else
            fail("Empty function body: void doNothing() { }");
    }
    else
        fail("Empty function body: void doNothing() { }");

    // Test 7: Function with return statement
    auto decl7 = parseDecl("int getAnswer() { x = 42; }");
    if (decl7 && decl7->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl7.get());
        if (func && func->getName() == "getAnswer" &&
            func->getReturnType() == "int" &&
            func->getBody() != nullptr)
            pass("Function with body: int getAnswer() { x = 42; }");
        else
            fail("Function with body: int getAnswer() { x = 42; }");
    }
    else
        fail("Function with body: int getAnswer() { x = 42; }");
}

void test_function_definitions()
{
    cout << "\n[TEST] Function Definitions (USER STORY #13)\n";

    // Test 1: Function with return statement - int add(int a, int b) { return a + b; }
    auto decl1 = parseDecl("int add(int a, int b) { return a + b; }");
    if (decl1 && decl1->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl1.get());
        if (func && func->getName() == "add" &&
            func->getReturnType() == "int" &&
            func->getParameters().size() == 2 &&
            func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 1)
            {
                Statement* stmt = body->getStatements()[0].get();
                if (stmt && stmt->getNodeType() == ASTNodeType::RETURN_STMT)
                {
                    ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt);
                    if (ret && ret->getReturnValue() != nullptr)
                        pass("Function with return: int add(int a, int b) { return a + b; }");
                    else
                        fail("Function with return - no return value");
                }
                else
                    fail("Function with return - not a return statement");
            }
            else
                fail("Function with return - wrong body size");
        }
        else
            fail("Function with return: int add(int a, int b) { return a + b; }");
    }
    else
        fail("Function with return: int add(int a, int b) { return a + b; }");

    // Test 2: Void function with return - void printHello() { return; }
    auto decl2 = parseDecl("void printHello() { return; }");
    if (decl2 && decl2->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl2.get());
        if (func && func->getName() == "printHello" &&
            func->getReturnType() == "void" &&
            func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 1)
            {
                Statement* stmt = body->getStatements()[0].get();
                if (stmt && stmt->getNodeType() == ASTNodeType::RETURN_STMT)
                {
                    ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt);
                    if (ret && ret->getReturnValue() == nullptr)
                        pass("Void return: void printHello() { return; }");
                    else
                        fail("Void return - has value when shouldn't");
                }
                else
                    fail("Void return - not a return statement");
            }
            else
                fail("Void return - wrong body size");
        }
        else
            fail("Void return: void printHello() { return; }");
    }
    else
        fail("Void return: void printHello() { return; }");

    // Test 3: Function with multiple statements and return
    auto decl3 = parseDecl("int calculate(int x, int y) { z = x + y; return z; }");
    if (decl3 && decl3->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl3.get());
        if (func && func->getName() == "calculate" &&
            func->getParameters().size() == 2 &&
            func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 2)
            {
                Statement* stmt2 = body->getStatements()[1].get();
                if (stmt2 && stmt2->getNodeType() == ASTNodeType::RETURN_STMT)
                    pass("Multiple statements with return");
                else
                    fail("Multiple statements with return");
            }
            else
                fail("Multiple statements with return - wrong count");
        }
        else
            fail("Multiple statements with return");
    }
    else
        fail("Multiple statements with return");

    // Test 4: Function with nested block and return
    auto decl4 = parseDecl("int max(int a, int b) { if (a > b) { return a; } return b; }");
    if (decl4 && decl4->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl4.get());
        if (func && func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 2)
            {
                // First statement is if
                Statement* if_stmt = body->getStatements()[0].get();
                if (if_stmt && if_stmt->getNodeType() == ASTNodeType::IF_STMT)
                {
                    // Second statement is return
                    Statement* ret_stmt = body->getStatements()[1].get();
                    if (ret_stmt && ret_stmt->getNodeType() == ASTNodeType::RETURN_STMT)
                        pass("Nested blocks with return");
                    else
                        fail("Nested blocks with return - second not return");
                }
                else
                    fail("Nested blocks with return - first not if");
            }
            else
                fail("Nested blocks with return - wrong count");
        }
        else
            fail("Nested blocks with return");
    }
    else
        fail("Nested blocks with return");

    // Test 5: Function returning expression
    auto decl5 = parseDecl("int square(int n) { return n * n; }");
    if (decl5 && decl5->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl5.get());
        if (func && func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 1)
            {
                Statement* stmt = body->getStatements()[0].get();
                if (stmt && stmt->getNodeType() == ASTNodeType::RETURN_STMT)
                {
                    ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt);
                    if (ret && ret->getReturnValue() != nullptr &&
                        ret->getReturnValue()->getNodeType() == ASTNodeType::BINARY_EXPR)
                        pass("Return expression: return n * n;");
                    else
                        fail("Return expression - not binary expr");
                }
                else
                    fail("Return expression - not return stmt");
            }
            else
                fail("Return expression - wrong count");
        }
        else
            fail("Return expression");
    }
    else
        fail("Return expression");

    // Test 6: Function with multiple returns (early exit)
    auto decl6 = parseDecl("int abs(int x) { if (x < 0) return -x; return x; }");
    if (decl6 && decl6->getNodeType() == ASTNodeType::FUNCTION_DECL)
    {
        FunctionDecl* func = dynamic_cast<FunctionDecl*>(decl6.get());
        if (func && func->getBody() != nullptr)
        {
            CompoundStmt* body = func->getBody();
            if (body && body->getStatements().size() == 2)
                pass("Multiple returns: int abs(int x) { if (x < 0) return -x; return x; }");
            else
                fail("Multiple returns - wrong count");
        }
        else
            fail("Multiple returns");
    }
    else
        fail("Multiple returns");
}

void test_function_calls()
{
    cout << "\n[TEST] Function Calls (USER STORY #14)\n";

    // Test 1: Simple function call with no arguments
    auto expr1 = parseExpr("getAnswer()");
    if (isCallExpr(expr1.get()))
    {
        CallExpr* call = dynamic_cast<CallExpr*>(expr1.get());
        if (call && isIdentifier(call->getCallee(), "getAnswer") &&
            call->getArguments().size() == 0)
            pass("No arguments: getAnswer()");
        else
            fail("No arguments: getAnswer()");
    }
    else
        fail("No arguments: getAnswer()");

    // Test 2: Function call with single argument
    auto expr2 = parseExpr("square(5)");
    if (isCallExpr(expr2.get()))
    {
        CallExpr* call = dynamic_cast<CallExpr*>(expr2.get());
        if (call && isIdentifier(call->getCallee(), "square") &&
            call->getArguments().size() == 1 &&
            isLiteral(call->getArguments()[0].get(), "5"))
            pass("Single argument: square(5)");
        else
            fail("Single argument: square(5)");
    }
    else
        fail("Single argument: square(5)");

    // Test 3: Function call with multiple arguments - add(3, 5)
    auto expr3 = parseExpr("add(3, 5)");
    if (isCallExpr(expr3.get()))
    {
        CallExpr* call = dynamic_cast<CallExpr*>(expr3.get());
        if (call && isIdentifier(call->getCallee(), "add") &&
            call->getArguments().size() == 2 &&
            isLiteral(call->getArguments()[0].get(), "3") &&
            isLiteral(call->getArguments()[1].get(), "5"))
            pass("Multiple arguments: add(3, 5)");
        else
            fail("Multiple arguments: add(3, 5)");
    }
    else
        fail("Multiple arguments: add(3, 5)");

    // Test 4: Function call with variable arguments
    auto expr4 = parseExpr("calculate(x, y, z)");
    if (isCallExpr(expr4.get()))
    {
        CallExpr* call = dynamic_cast<CallExpr*>(expr4.get());
        if (call && isIdentifier(call->getCallee(), "calculate") &&
            call->getArguments().size() == 3 &&
            isIdentifier(call->getArguments()[0].get(), "x") &&
            isIdentifier(call->getArguments()[1].get(), "y") &&
            isIdentifier(call->getArguments()[2].get(), "z"))
            pass("Variable arguments: calculate(x, y, z)");
        else
            fail("Variable arguments: calculate(x, y, z)");
    }
    else
        fail("Variable arguments: calculate(x, y, z)");

    // Test 5: Function call as expression in assignment - x = add(3, 5);
    auto stmt1 = parseStmt("x = add(3, 5);");
    if (stmt1 && stmt1->getNodeType() == ASTNodeType::EXPRESSION_STMT)
    {
        ExpressionStmt* exprStmt = dynamic_cast<ExpressionStmt*>(stmt1.get());
        if (exprStmt && isAssignment(exprStmt->getExpression()))
        {
            AssignmentExpr* assign = dynamic_cast<AssignmentExpr*>(exprStmt->getExpression());
            if (assign && isIdentifier(assign->getTarget(), "x") &&
                isCallExpr(assign->getValue()))
            {
                CallExpr* call = dynamic_cast<CallExpr*>(assign->getValue());
                if (call && isIdentifier(call->getCallee(), "add") &&
                    call->getArguments().size() == 2)
                    pass("Call in assignment: x = add(3, 5);");
                else
                    fail("Call in assignment - bad call");
            }
            else
                fail("Call in assignment - bad structure");
        }
        else
            fail("Call in assignment - not assignment");
    }
    else
        fail("Call in assignment: x = add(3, 5);");

    // Test 6: Function call with expression arguments
    auto expr5 = parseExpr("max(a + b, c * d)");
    if (isCallExpr(expr5.get()))
    {
        CallExpr* call = dynamic_cast<CallExpr*>(expr5.get());
        if (call && isIdentifier(call->getCallee(), "max") &&
            call->getArguments().size() == 2 &&
            isBinaryOp(call->getArguments()[0].get(), "+") &&
            isBinaryOp(call->getArguments()[1].get(), "*"))
            pass("Expression arguments: max(a + b, c * d)");
        else
            fail("Expression arguments: max(a + b, c * d)");
    }
    else
        fail("Expression arguments: max(a + b, c * d)");

    // Test 7: Nested function calls - add(square(3), square(4))
    auto expr6 = parseExpr("add(square(3), square(4))");
    if (isCallExpr(expr6.get()))
    {
        CallExpr* outer = dynamic_cast<CallExpr*>(expr6.get());
        if (outer && isIdentifier(outer->getCallee(), "add") &&
            outer->getArguments().size() == 2 &&
            isCallExpr(outer->getArguments()[0].get()) &&
            isCallExpr(outer->getArguments()[1].get()))
        {
            CallExpr* call1 = dynamic_cast<CallExpr*>(outer->getArguments()[0].get());
            CallExpr* call2 = dynamic_cast<CallExpr*>(outer->getArguments()[1].get());
            if (call1 && isIdentifier(call1->getCallee(), "square") &&
                call2 && isIdentifier(call2->getCallee(), "square"))
                pass("Nested calls: add(square(3), square(4))");
            else
                fail("Nested calls - inner calls bad");
        }
        else
            fail("Nested calls - outer call bad");
    }
    else
        fail("Nested calls: add(square(3), square(4))");

    // Test 8: Function call in binary expression
    auto expr7 = parseExpr("x + getValue()");
    if (isBinaryOp(expr7.get(), "+"))
    {
        BinaryExpr* bin = dynamic_cast<BinaryExpr*>(expr7.get());
        if (bin && isIdentifier(bin->getLeft(), "x") &&
            isCallExpr(bin->getRight()))
        {
            CallExpr* call = dynamic_cast<CallExpr*>(bin->getRight());
            if (call && isIdentifier(call->getCallee(), "getValue"))
                pass("Call in expression: x + getValue()");
            else
                fail("Call in expression - bad call");
        }
        else
            fail("Call in expression - bad binary");
    }
    else
        fail("Call in expression: x + getValue()");
}

void test_return_statements()
{
    cout << "\n[TEST] Return Statements (USER STORY #15)\n";

    // Test 1: Return with expression - return x;
    auto stmt1 = parseStmt("return x;");
    if (stmt1 && stmt1->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt1.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isIdentifier(ret->getReturnValue(), "x"))
            pass("Return variable: return x;");
        else
            fail("Return variable: return x;");
    }
    else
        fail("Return variable: return x;");

    // Test 2: Return literal - return 42;
    auto stmt2 = parseStmt("return 42;");
    if (stmt2 && stmt2->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt2.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isLiteral(ret->getReturnValue(), "42"))
            pass("Return literal: return 42;");
        else
            fail("Return literal: return 42;");
    }
    else
        fail("Return literal: return 42;");

    // Test 3: Return expression - return a + b;
    auto stmt3 = parseStmt("return a + b;");
    if (stmt3 && stmt3->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt3.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isBinaryOp(ret->getReturnValue(), "+"))
            pass("Return expression: return a + b;");
        else
            fail("Return expression: return a + b;");
    }
    else
        fail("Return expression: return a + b;");

    // Test 4: Return complex expression - return x * y + z;
    auto stmt4 = parseStmt("return x * y + z;");
    if (stmt4 && stmt4->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt4.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isBinaryOp(ret->getReturnValue(), "+"))
        {
            BinaryExpr* bin = dynamic_cast<BinaryExpr*>(ret->getReturnValue());
            if (bin && isBinaryOp(bin->getLeft(), "*"))
                pass("Return complex expression: return x * y + z;");
            else
                fail("Return complex expression - bad structure");
        }
        else
            fail("Return complex expression: return x * y + z;");
    }
    else
        fail("Return complex expression: return x * y + z;");

    // Test 5: Void return - return;
    auto stmt5 = parseStmt("return;");
    if (stmt5 && stmt5->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt5.get());
        if (ret && ret->getReturnValue() == nullptr)
            pass("Void return: return;");
        else
            fail("Void return: return; - has value when shouldn't");
    }
    else
        fail("Void return: return;");

    // Test 6: Return function call - return getValue();
    auto stmt6 = parseStmt("return getValue();");
    if (stmt6 && stmt6->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt6.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isCallExpr(ret->getReturnValue()))
        {
            CallExpr* call = dynamic_cast<CallExpr*>(ret->getReturnValue());
            if (call && isIdentifier(call->getCallee(), "getValue"))
                pass("Return function call: return getValue();");
            else
                fail("Return function call - bad call");
        }
        else
            fail("Return function call: return getValue();");
    }
    else
        fail("Return function call: return getValue();");

    // Test 7: Return nested call - return add(square(x), 5);
    auto stmt7 = parseStmt("return add(square(x), 5);");
    if (stmt7 && stmt7->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt7.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isCallExpr(ret->getReturnValue()))
        {
            CallExpr* outer = dynamic_cast<CallExpr*>(ret->getReturnValue());
            if (outer && isIdentifier(outer->getCallee(), "add") &&
                outer->getArguments().size() == 2 &&
                isCallExpr(outer->getArguments()[0].get()))
                pass("Return nested call: return add(square(x), 5);");
            else
                fail("Return nested call - bad structure");
        }
        else
            fail("Return nested call: return add(square(x), 5);");
    }
    else
        fail("Return nested call: return add(square(x), 5);");

    // Test 8: Return unary expression - return -x;
    auto stmt8 = parseStmt("return -x;");
    if (stmt8 && stmt8->getNodeType() == ASTNodeType::RETURN_STMT)
    {
        ReturnStmt* ret = dynamic_cast<ReturnStmt*>(stmt8.get());
        if (ret && ret->getReturnValue() != nullptr &&
            isUnaryOp(ret->getReturnValue(), "-"))
            pass("Return unary expression: return -x;");
        else
            fail("Return unary expression: return -x;");
    }
    else
        fail("Return unary expression: return -x;");
}

void test_array_declarations()
{
    cout << "\n[TEST] Array Declarations (USER STORY #16)\n";

    // Test 1: Simple array declaration - int arr[10];
    auto decl1 = parseDecl("int arr[10];");
    if (decl1 && decl1->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl1.get());
        if (var && var->getName() == "arr" &&
            var->getType() == "int" &&
            var->getIsArray() &&
            var->getArraySize() != nullptr &&
            isLiteral(var->getArraySize(), "10"))
            pass("Simple array: int arr[10];");
        else
            fail("Simple array: int arr[10];");
    }
    else
        fail("Simple array: int arr[10];");

    // Test 2: Char array - char str[50];
    auto decl2 = parseDecl("char str[50];");
    if (decl2 && decl2->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl2.get());
        if (var && var->getName() == "str" &&
            var->getType() == "char" &&
            var->getIsArray() &&
            var->getArraySize() != nullptr &&
            isLiteral(var->getArraySize(), "50"))
            pass("Char array: char str[50];");
        else
            fail("Char array: char str[50];");
    }
    else
        fail("Char array: char str[50];");

    // Test 3: Float array - float numbers[100];
    auto decl3 = parseDecl("float numbers[100];");
    if (decl3 && decl3->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl3.get());
        if (var && var->getName() == "numbers" &&
            var->getType() == "float" &&
            var->getIsArray() &&
            var->getArraySize() != nullptr &&
            isLiteral(var->getArraySize(), "100"))
            pass("Float array: float numbers[100];");
        else
            fail("Float array: float numbers[100];");
    }
    else
        fail("Float array: float numbers[100];");

    // Test 4: Array with expression size - int data[n];
    auto decl4 = parseDecl("int data[n];");
    if (decl4 && decl4->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl4.get());
        if (var && var->getName() == "data" &&
            var->getType() == "int" &&
            var->getIsArray() &&
            var->getArraySize() != nullptr &&
            isIdentifier(var->getArraySize(), "n"))
            pass("Variable size: int data[n];");
        else
            fail("Variable size: int data[n];");
    }
    else
        fail("Variable size: int data[n];");

    // Test 5: Array with expression size - int buffer[SIZE * 2];
    auto decl5 = parseDecl("int buffer[SIZE * 2];");
    if (decl5 && decl5->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl5.get());
        if (var && var->getName() == "buffer" &&
            var->getType() == "int" &&
            var->getIsArray() &&
            var->getArraySize() != nullptr &&
            isBinaryOp(var->getArraySize(), "*"))
            pass("Expression size: int buffer[SIZE * 2];");
        else
            fail("Expression size: int buffer[SIZE * 2];");
    }
    else
        fail("Expression size: int buffer[SIZE * 2];");

    // Test 6: Different types
    auto decl6 = parseDecl("double values[20];");
    if (decl6 && decl6->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl6.get());
        if (var && var->getName() == "values" &&
            var->getType() == "double" &&
            var->getIsArray())
            pass("Double array: double values[20];");
        else
            fail("Double array: double values[20];");
    }
    else
        fail("Double array: double values[20];");

    // Test 7: Regular variable still works (not array)
    auto decl7 = parseDecl("int x;");
    if (decl7 && decl7->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl7.get());
        if (var && var->getName() == "x" &&
            var->getType() == "int" &&
            !var->getIsArray() &&
            var->getArraySize() == nullptr)
            pass("Regular variable (not array): int x;");
        else
            fail("Regular variable (not array): int x;");
    }
    else
        fail("Regular variable (not array): int x;");
}

void test_array_access()
{
    cout << "\n[TEST] Array Access (USER STORY #17)\n";

    // Test 1: Simple array access - arr[index]
    auto expr1 = parseExpr("arr[index]");
    if (isArrayAccess(expr1.get()))
    {
        ArrayAccessExpr* access = dynamic_cast<ArrayAccessExpr*>(expr1.get());
        if (access && isIdentifier(access->getArray(), "arr") &&
            isIdentifier(access->getIndex(), "index"))
            pass("Simple access: arr[index]");
        else
            fail("Simple access: arr[index]");
    }
    else
        fail("Simple access: arr[index]");

    // Test 2: Array access with literal index - arr[0]
    auto expr2 = parseExpr("arr[0]");
    if (isArrayAccess(expr2.get()))
    {
        ArrayAccessExpr* access = dynamic_cast<ArrayAccessExpr*>(expr2.get());
        if (access && isIdentifier(access->getArray(), "arr") &&
            isLiteral(access->getIndex(), "0"))
            pass("Literal index: arr[0]");
        else
            fail("Literal index: arr[0]");
    }
    else
        fail("Literal index: arr[0]");

    // Test 3: Array access with expression index - arr[i + 1]
    auto expr3 = parseExpr("arr[i + 1]");
    if (isArrayAccess(expr3.get()))
    {
        ArrayAccessExpr* access = dynamic_cast<ArrayAccessExpr*>(expr3.get());
        if (access && isIdentifier(access->getArray(), "arr") &&
            isBinaryOp(access->getIndex(), "+"))
            pass("Expression index: arr[i + 1]");
        else
            fail("Expression index: arr[i + 1]");
    }
    else
        fail("Expression index: arr[i + 1]");

    // Test 4: Chained array access (multidimensional) - matrix[i][j]
    auto expr4 = parseExpr("matrix[i][j]");
    if (isArrayAccess(expr4.get()))
    {
        ArrayAccessExpr* outer = dynamic_cast<ArrayAccessExpr*>(expr4.get());
        if (outer && isArrayAccess(outer->getArray()) &&
            isIdentifier(outer->getIndex(), "j"))
        {
            ArrayAccessExpr* inner = dynamic_cast<ArrayAccessExpr*>(outer->getArray());
            if (inner && isIdentifier(inner->getArray(), "matrix") &&
                isIdentifier(inner->getIndex(), "i"))
                pass("Chained access: matrix[i][j]");
            else
                fail("Chained access - inner bad");
        }
        else
            fail("Chained access - outer bad");
    }
    else
        fail("Chained access: matrix[i][j]");

    // Test 5: Array access as lvalue - arr[i] = 5;
    auto stmt1 = parseStmt("arr[i] = 5;");
    if (stmt1 && stmt1->getNodeType() == ASTNodeType::EXPRESSION_STMT)
    {
        ExpressionStmt* exprStmt = dynamic_cast<ExpressionStmt*>(stmt1.get());
        if (exprStmt && isAssignment(exprStmt->getExpression()))
        {
            AssignmentExpr* assign = dynamic_cast<AssignmentExpr*>(exprStmt->getExpression());
            if (assign && isArrayAccess(assign->getTarget()) &&
                isLiteral(assign->getValue(), "5"))
            {
                ArrayAccessExpr* access = dynamic_cast<ArrayAccessExpr*>(assign->getTarget());
                if (access && isIdentifier(access->getArray(), "arr") &&
                    isIdentifier(access->getIndex(), "i"))
                    pass("Array as lvalue: arr[i] = 5;");
                else
                    fail("Array as lvalue - bad access");
            }
            else
                fail("Array as lvalue - bad assignment");
        }
        else
            fail("Array as lvalue - not assignment");
    }
    else
        fail("Array as lvalue: arr[i] = 5;");

    // Test 6: Array access in expression - x = arr[i] + 10;
    auto stmt2 = parseStmt("x = arr[i] + 10;");
    if (stmt2 && stmt2->getNodeType() == ASTNodeType::EXPRESSION_STMT)
    {
        ExpressionStmt* exprStmt = dynamic_cast<ExpressionStmt*>(stmt2.get());
        if (exprStmt && isAssignment(exprStmt->getExpression()))
        {
            AssignmentExpr* assign = dynamic_cast<AssignmentExpr*>(exprStmt->getExpression());
            if (assign && isBinaryOp(assign->getValue(), "+"))
            {
                BinaryExpr* bin = dynamic_cast<BinaryExpr*>(assign->getValue());
                if (bin && isArrayAccess(bin->getLeft()))
                    pass("Array in expression: x = arr[i] + 10;");
                else
                    fail("Array in expression - bad binary");
            }
            else
                fail("Array in expression - bad assignment");
        }
        else
            fail("Array in expression - not assignment");
    }
    else
        fail("Array in expression: x = arr[i] + 10;");

    // Test 7: Array of function results - results[getValue()]
    auto expr5 = parseExpr("results[getValue()]");
    if (isArrayAccess(expr5.get()))
    {
        ArrayAccessExpr* access = dynamic_cast<ArrayAccessExpr*>(expr5.get());
        if (access && isIdentifier(access->getArray(), "results") &&
            isCallExpr(access->getIndex()))
            pass("Function as index: results[getValue()]");
        else
            fail("Function as index: results[getValue()]");
    }
    else
        fail("Function as index: results[getValue()]");

    // Test 8: Complex index expression - arr[i * 2 + offset]
    auto expr6 = parseExpr("arr[i * 2 + offset]");
    if (isArrayAccess(expr6.get()))
    {
        ArrayAccessExpr* access = dynamic_cast<ArrayAccessExpr*>(expr6.get());
        if (access && isIdentifier(access->getArray(), "arr") &&
            isBinaryOp(access->getIndex(), "+"))
        {
            BinaryExpr* idx = dynamic_cast<BinaryExpr*>(access->getIndex());
            if (idx && isBinaryOp(idx->getLeft(), "*"))
                pass("Complex index: arr[i * 2 + offset]");
            else
                fail("Complex index - bad structure");
        }
        else
            fail("Complex index: arr[i * 2 + offset]");
    }
    else
        fail("Complex index: arr[i * 2 + offset]");
}

void test_pointer_declarations()
{
    cout << "\n[TEST] Pointer Declarations (USER STORY #18)\n";

    // Test 1: Simple pointer declaration - int *ptr;
    auto decl1 = parseDecl("int *ptr;");
    if (decl1 && decl1->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl1.get());
        if (var && var->getName() == "ptr" &&
            var->getType() == "int" &&
            var->isPointer() &&
            var->getPointerLevel() == 1)
            pass("Simple pointer: int *ptr;");
        else
            fail("Simple pointer: int *ptr;");
    }
    else
        fail("Simple pointer: int *ptr;");

    // Test 2: Char pointer - char *str;
    auto decl2 = parseDecl("char *str;");
    if (decl2 && decl2->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl2.get());
        if (var && var->getName() == "str" &&
            var->getType() == "char" &&
            var->isPointer() &&
            var->getPointerLevel() == 1)
            pass("Char pointer: char *str;");
        else
            fail("Char pointer: char *str;");
    }
    else
        fail("Char pointer: char *str;");

    // Test 3: Double pointer - int **pptr;
    auto decl3 = parseDecl("int **pptr;");
    if (decl3 && decl3->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl3.get());
        if (var && var->getName() == "pptr" &&
            var->getType() == "int" &&
            var->isPointer() &&
            var->getPointerLevel() == 2)
            pass("Double pointer: int **pptr;");
        else
            fail("Double pointer: int **pptr;");
    }
    else
        fail("Double pointer: int **pptr;");

    // Test 4: Triple pointer - char ***ppptr;
    auto decl4 = parseDecl("char ***ppptr;");
    if (decl4 && decl4->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl4.get());
        if (var && var->getName() == "ppptr" &&
            var->getType() == "char" &&
            var->isPointer() &&
            var->getPointerLevel() == 3)
            pass("Triple pointer: char ***ppptr;");
        else
            fail("Triple pointer: char ***ppptr;");
    }
    else
        fail("Triple pointer: char ***ppptr;");

    // Test 5: Float pointer - float *fptr;
    auto decl5 = parseDecl("float *fptr;");
    if (decl5 && decl5->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl5.get());
        if (var && var->getName() == "fptr" &&
            var->getType() == "float" &&
            var->isPointer() &&
            var->getPointerLevel() == 1)
            pass("Float pointer: float *fptr;");
        else
            fail("Float pointer: float *fptr;");
    }
    else
        fail("Float pointer: float *fptr;");

    // Test 6: Void pointer - void *vptr;
    auto decl6 = parseDecl("void *vptr;");
    if (decl6 && decl6->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl6.get());
        if (var && var->getName() == "vptr" &&
            var->getType() == "void" &&
            var->isPointer() &&
            var->getPointerLevel() == 1)
            pass("Void pointer: void *vptr;");
        else
            fail("Void pointer: void *vptr;");
    }
    else
        fail("Void pointer: void *vptr;");

    // Test 7: Regular variable still works (not a pointer)
    auto decl7 = parseDecl("int x;");
    if (decl7 && decl7->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl7.get());
        if (var && var->getName() == "x" &&
            var->getType() == "int" &&
            !var->isPointer() &&
            var->getPointerLevel() == 0)
            pass("Regular variable (not pointer): int x;");
        else
            fail("Regular variable (not pointer): int x;");
    }
    else
        fail("Regular variable (not pointer): int x;");

    // Test 8: Pointer with initializer - int *ptr = x;
    auto decl8 = parseDecl("int *ptr = x;");
    if (decl8 && decl8->getNodeType() == ASTNodeType::VAR_DECL)
    {
        VarDecl* var = dynamic_cast<VarDecl*>(decl8.get());
        if (var && var->getName() == "ptr" &&
            var->getType() == "int" &&
            var->isPointer() &&
            var->getPointerLevel() == 1 &&
            var->getInitializer() != nullptr)
            pass("Pointer with initializer: int *ptr = x;");
        else
            fail("Pointer with initializer: int *ptr = x;");
    }
    else
        fail("Pointer with initializer: int *ptr = x;");
}

void test_precedence()
{
    cout << "\n[TEST] Operator Precedence\n";

    auto expr1 = parseExpr("a + b * c");
    if (isBinaryOp(expr1.get(), "+") &&
        isIdentifier(left(expr1.get()), "a") &&
        isBinaryOp(right(expr1.get()), "*"))
        pass("* has higher precedence than +");
    else
        fail("* has higher precedence than +");

    auto expr2 = parseExpr("a * b + c");
    if (isBinaryOp(expr2.get(), "+") &&
        isBinaryOp(left(expr2.get()), "*") &&
        isIdentifier(right(expr2.get()), "c"))
        pass("Left associativity with different precedence");
    else
        fail("Left associativity with different precedence");
}

void test_associativity()
{
    cout << "\n[TEST] Left-to-Right Associativity\n";

    auto expr = parseExpr("a - b - c");
    if (isBinaryOp(expr.get(), "-") &&
        isBinaryOp(left(expr.get()), "-") &&
        isIdentifier(right(expr.get()), "c"))
        pass("a - b - c is ((a - b) - c)");
    else
        fail("a - b - c is ((a - b) - c)");
}

void test_all_operators()
{
    cout << "\n[TEST] All Operators\n";

    string ops[][2] = {
        {"a + b", "+"}, {"a - b", "-"}, {"a * b", "*"}, {"a / b", "/"}, {"a % b", "%"},
        {"a == b", "=="}, {"a != b", "!="}, {"a < b", "<"}, {"a > b", ">"},
        {"a <= b", "<="}, {"a >= b", ">="}, {"a && b", "&&"}
    };

    int passed = 0;
    for (auto& test : ops) {
        auto expr = parseExpr(test[0]);
        if (isBinaryOp(expr.get(), test[1]))
            passed++;
    }

    if (passed == 12)
        pass("All 12 binary operators work");
    else
        fail("Some operators failed (" + to_string(passed) + "/12)");
}

void test_complex_expressions()
{
    cout << "\n[TEST] Complex Expressions\n";

    auto expr1 = parseExpr("5 + 10 * 2");
    if (isBinaryOp(expr1.get(), "+") &&
        isLiteral(left(expr1.get()), "5") &&
        isBinaryOp(right(expr1.get()), "*"))
        pass("Literals with operators: 5 + 10 * 2");
    else
        fail("Literals with operators: 5 + 10 * 2");

    auto expr2 = parseExpr("a + b * c - d");
    if (isBinaryOp(expr2.get(), "-") &&
        isBinaryOp(left(expr2.get()), "+") &&
        isIdentifier(right(expr2.get()), "d"))
        pass("Multiple operators: a + b * c - d");
    else
        fail("Multiple operators: a + b * c - d");

    auto expr3 = parseExpr("-a + b");
    if (isBinaryOp(expr3.get(), "+") &&
        isUnaryOp(left(expr3.get()), "-"))
        pass("Unary in binary expression: -a + b");
    else
        fail("Unary in binary expression: -a + b");
}

void test_precedence_levels()
{
    cout << "\n[TEST] All Precedence Levels\n";

    auto expr = parseExpr("a && b == c + d * e");

    bool correct =
        isBinaryOp(expr.get(), "&&") &&                     // Root is &&
        isBinaryOp(right(expr.get()), "==") &&              // Right is ==
        isBinaryOp(right(right(expr.get())), "+") &&        // Right-right is +
        isBinaryOp(right(right(right(expr.get()))), "*");   // Right-right-right is *

    if (correct)
        pass("All precedence levels: && < == < + < *");
    else
        fail("All precedence levels: && < == < + < *");
}

void test_comparison_operators()
{
    cout << "\n[TEST] Comparison with Logical Operators\n";

    auto expr = parseExpr("a < b && c > d");
    if (isBinaryOp(expr.get(), "&&") &&
        isBinaryOp(left(expr.get()), "<") &&
        isBinaryOp(right(expr.get()), ">"))
        pass("Comparison operators bind tighter than &&");
    else
        fail("Comparison operators bind tighter than &&");
}

// ============================================================================
// Main
// ============================================================================

int main()
{
    cout << "========================================" << endl;
    cout << "C COMPILER PARSER TESTS" << endl;
    cout << "========================================" << endl;

    // Expression tests
    test_identifiers();
    test_literals();
    test_binary_ops();
    test_unary_ops();
    test_parentheses();
    test_assignments();
    test_function_calls();

    // Declaration tests
    test_variable_declarations();
    test_array_declarations();
    test_pointer_declarations();
    test_array_access();
    test_function_declarations();
    test_function_definitions();

    // Statement tests
    test_if_statements();
    test_while_loops();
    test_for_loops();
    test_return_statements();
    test_compound_statements();

    // Precedence tests
    test_precedence();
    test_associativity();
    test_all_operators();
    test_complex_expressions();
    test_precedence_levels();
    test_comparison_operators();

    cout << "\n========================================" << endl;
    cout << "TESTS COMPLETE" << endl;
    cout << "========================================" << endl;

    return 0;
}
