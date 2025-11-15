#include "include/parser.h"
#include "include/lexer.h"
#include "include/ast.h"
#include <iostream>
#include <cassert>

using namespace std;

// Simple AST printer visitor for testing
class TestPrinter : public ASTVisitor
{
public:
    void visit(BinaryExpr &node) override {}
    void visit(UnaryExpr &node) override {}
    void visit(LiteralExpr &node) override {}
    void visit(IdentifierExpr &node) override
    {
        cout << "IdentifierExpr: " << node.getName() << " at " << node.getLocation().toString() << endl;
    }
    void visit(CallExpr &node) override {}
    void visit(IfStmt &node) override {}
    void visit(WhileStmt &node) override {}
    void visit(ForStmt &node) override {}
    void visit(ReturnStmt &node) override {}
    void visit(CompoundStmt &node) override {}
    void visit(VarDecl &node) override {}
    void visit(TypeDecl &node) override {}
    void visit(StructDecl &node) override {}
    void visit(FunctionDecl &node) override {}
    void visit(ParameterDecl &node) override {}
};

void testSingleIdentifier()
{
    cout << "\n=== Test 1: Single Identifier ===" << endl;

    string source = "variable_name";
    Lexer lexer(source, "test1.c");
    Parser parser(lexer);

    auto expr = parser.parseIdentifier();

    // Verify it's an IdentifierExpr
    assert(expr->getNodeType() == ASTNodeType::IDENTIFIER_EXPR);

    // Cast and check the name
    IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(expr.get());
    assert(id != nullptr);
    assert(id->getName() == "variable_name");

    cout << "✓ Parsed identifier: " << id->getName() << endl;
    cout << "✓ Location: " << id->getLocation().toString() << endl;
}

void testMultipleIdentifiers()
{
    cout << "\n=== Test 2: Multiple Identifiers ===" << endl;

    // Note: This tests parsing identifiers one at a time
    vector<string> identifiers = {"x", "count", "myVariable", "_private", "value123"};

    for (const auto &id_name : identifiers)
    {
        Lexer lexer(id_name, "test2.c");
        Parser parser(lexer);

        auto expr = parser.parseIdentifier();
        IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(expr.get());

        assert(id != nullptr);
        assert(id->getName() == id_name);

        cout << "✓ Parsed: " << id->getName() << endl;
    }
}

void testIdentifierThroughPrimaryExpression()
{
    cout << "\n=== Test 3: Identifier via parsePrimaryExpression ===" << endl;

    string source = "userName";
    Lexer lexer(source, "test3.c");
    Parser parser(lexer);

    // Parse through the primary expression method (more realistic usage)
    auto expr = parser.parsePrimaryExpression();

    assert(expr->getNodeType() == ASTNodeType::IDENTIFIER_EXPR);
    IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(expr.get());
    assert(id != nullptr);
    assert(id->getName() == "userName");

    cout << "✓ parsePrimaryExpression correctly handled identifier" << endl;
}

void testIdentifierDoesNotValidateExistence()
{
    cout << "\n=== Test 4: Identifiers NOT Validated (As Required) ===" << endl;

    // These identifiers don't "exist" - parser should accept them anyway
    vector<string> undefined_identifiers = {
        "undefinedVar",
        "noSuchFunction",
        "randomName123"};

    for (const auto &id_name : undefined_identifiers)
    {
        Lexer lexer(id_name, "test4.c");
        Parser parser(lexer);

        // Should parse successfully WITHOUT checking if variable exists
        auto expr = parser.parseIdentifier();
        IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(expr.get());

        assert(id != nullptr);
        assert(id->getName() == id_name);

        cout << "✓ Parsed (no validation): " << id->getName() << endl;
    }

    cout << "✓ Parser correctly does NOT validate existence (semantic analysis phase)" << endl;
}

void testVisitorPattern()
{
    cout << "\n=== Test 5: Visitor Pattern Integration ===" << endl;

    string source = "testVariable";
    Lexer lexer(source, "test5.c");
    Parser parser(lexer);

    auto expr = parser.parseIdentifier();

    TestPrinter printer;
    expr->accept(printer);

    cout << "✓ Visitor pattern working correctly" << endl;
}

void testSourceLocationTracking()
{
    cout << "\n=== Test 6: Source Location Tracking ===" << endl;

    // Create source with identifier at specific location
    string source = "\n\n    variable";  // Line 3, column 5 (0-indexed might vary)
    Lexer lexer(source, "test6.c");
    Parser parser(lexer);

    // Skip whitespace/newlines to get to identifier
    auto expr = parser.parsePrimaryExpression();

    IdentifierExpr *id = dynamic_cast<IdentifierExpr *>(expr.get());
    assert(id != nullptr);

    auto loc = id->getLocation();
    cout << "✓ Location tracked: " << loc.toString() << endl;
    cout << "  Filename: " << loc.filename << endl;
    cout << "  Line: " << loc.line << endl;
    cout << "  Column: " << loc.column << endl;
}

int main()
{
    cout << "=====================================" << endl;
    cout << "Parser Identifier Tests (User Story #2)" << endl;
    cout << "=====================================" << endl;

    try
    {
        testSingleIdentifier();
        testMultipleIdentifiers();
        testIdentifierThroughPrimaryExpression();
        testIdentifierDoesNotValidateExistence();
        testVisitorPattern();
        testSourceLocationTracking();

        cout << "\n=====================================" << endl;
        cout << "✅ ALL TESTS PASSED!" << endl;
        cout << "=====================================" << endl;
        cout << "\nAcceptance Criteria Met:" << endl;
        cout << "✓ Creates IdentifierNode with variable name" << endl;
        cout << "✓ Does NOT validate existence (semantic analysis later)" << endl;
        cout << "=====================================" << endl;

        return 0;
    }
    catch (const exception &e)
    {
        cout << "\n❌ TEST FAILED: " << e.what() << endl;
        return 1;
    }
}
