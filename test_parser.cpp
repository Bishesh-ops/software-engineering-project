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
    cout << "\n[TEST] Compound Statements\n";

    auto stmt = parseStmt("{ x = 1; y = 2; }");
    if (stmt && stmt->getNodeType() == ASTNodeType::COMPOUND_STMT)
    {
        CompoundStmt* compound = dynamic_cast<CompoundStmt*>(stmt.get());
        if (compound && compound->getStatements().size() == 2)
            pass("Block with 2 statements: { x = 1; y = 2; }");
        else
            fail("Block with 2 statements: { x = 1; y = 2; }");
    }
    else
        fail("Block with 2 statements: { x = 1; y = 2; }");
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

    // Declaration tests
    test_variable_declarations();

    // Statement tests
    test_if_statements();
    test_while_loops();
    test_for_loops();
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
