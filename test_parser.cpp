#include "include/parser.h"
#include "include/lexer.h"
#include "include/ast.h"
#include <iostream>
#include <string>

using namespace std;

// ============================================================================
// Simple Helper Functions
// ============================================================================

unique_ptr<Expression> parse(const string& source)
{
    Lexer lexer(source, "test.c");
    Parser parser(lexer);
    return parser.parseExpression();
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

void pass(const string& msg) { cout << "  ✓ " << msg << endl; }
void fail(const string& msg) { cout << "  ✗ " << msg << endl; }

// ============================================================================
// Tests
// ============================================================================

void test_identifiers()
{
    cout << "\n[TEST] Identifiers\n";

    auto expr1 = parse("myVariable");
    if (isIdentifier(expr1.get(), "myVariable"))
        pass("Simple identifier");
    else
        fail("Simple identifier");

    auto expr2 = parse("_private123");
    if (isIdentifier(expr2.get(), "_private123"))
        pass("Identifier with underscore and numbers");
    else
        fail("Identifier with underscore and numbers");
}

void test_literals()
{
    cout << "\n[TEST] Literals\n";

    auto expr1 = parse("42");
    if (isLiteral(expr1.get(), "42"))
        pass("Integer literal");
    else
        fail("Integer literal");

    auto expr2 = parse("3.14");
    if (isLiteral(expr2.get(), "3.14"))
        pass("Float literal");
    else
        fail("Float literal");
}

void test_simple_binary_ops()
{
    cout << "\n[TEST] Simple Binary Operations\n";

    auto expr = parse("a + b");
    if (isBinaryOp(expr.get(), "+") &&
        isIdentifier(left(expr.get()), "a") &&
        isIdentifier(right(expr.get()), "b"))
        pass("a + b");
    else
        fail("a + b");

    auto expr2 = parse("x * y");
    if (isBinaryOp(expr2.get(), "*") &&
        isIdentifier(left(expr2.get()), "x") &&
        isIdentifier(right(expr2.get()), "y"))
        pass("x * y");
    else
        fail("x * y");
}

void test_precedence()
{
    cout << "\n[TEST] Operator Precedence\n";

    // Test: a + b * c should be (a + (b * c))
    auto expr1 = parse("a + b * c");
    if (isBinaryOp(expr1.get(), "+") &&
        isIdentifier(left(expr1.get()), "a") &&
        isBinaryOp(right(expr1.get()), "*"))
        pass("* has higher precedence than +");
    else
        fail("* has higher precedence than +");

    // Test: a * b + c should be ((a * b) + c)
    auto expr2 = parse("a * b + c");
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

    // Test: a - b - c should be ((a - b) - c)
    auto expr = parse("a - b - c");
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
        auto expr = parse(test[0]);
        if (isBinaryOp(expr.get(), test[1]))
            passed++;
    }

    if (passed == 12)
        pass("All 12 operators work");
    else
        fail("Some operators failed (" + to_string(passed) + "/12)");
}

void test_complex_expressions()
{
    cout << "\n[TEST] Complex Expressions\n";

    // Test: 5 + 10 * 2 should be (5 + (10 * 2))
    auto expr1 = parse("5 + 10 * 2");
    if (isBinaryOp(expr1.get(), "+") &&
        isLiteral(left(expr1.get()), "5") &&
        isBinaryOp(right(expr1.get()), "*"))
        pass("Literals with operators: 5 + 10 * 2");
    else
        fail("Literals with operators: 5 + 10 * 2");

    // Test: a + b * c - d should be ((a + (b * c)) - d)
    auto expr2 = parse("a + b * c - d");
    if (isBinaryOp(expr2.get(), "-") &&
        isBinaryOp(left(expr2.get()), "+") &&
        isIdentifier(right(expr2.get()), "d"))
        pass("Multiple operators: a + b * c - d");
    else
        fail("Multiple operators: a + b * c - d");
}

void test_precedence_levels()
{
    cout << "\n[TEST] All Precedence Levels\n";

    // Test: a && b == c + d * e
    // Should be: (a && (b == (c + (d * e))))
    auto expr = parse("a && b == c + d * e");

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

    // Test: a < b && c > d should be ((a < b) && (c > d))
    auto expr = parse("a < b && c > d");
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

    test_identifiers();
    test_literals();
    test_simple_binary_ops();
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
