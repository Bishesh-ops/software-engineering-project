#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic_analyzer.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    cout << "========================================\n";
    cout << "Temporary Test - Warning System\n";
    cout << "========================================\n\n";

    // Test 1: Warnings enabled (default)
    cout << "Test 1: Warnings ENABLED\n";
    cout << "-------------------------\n";
    {
        string code = R"(
            int test() {
                int x = 5;
                float y = 3.14;
                x = y;  // Should warn about float to int conversion
            }
        )";

        Lexer lexer(code, "test.c");
        Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.hasErrors()) {
            SemanticAnalyzer analyzer;
            analyzer.set_warnings_enabled(true);  // Enable warnings
            analyzer.analyze_program(program);

            cout << "Errors: " << analyzer.getErrorHandler().get_error_count() << "\n";
            cout << "Warnings: " << analyzer.getErrorHandler().get_warning_count() << "\n";

            if (analyzer.getErrorHandler().get_warning_count() > 0) {
                cout << "[PASS] Warning was emitted\n";
            } else {
                cout << "[FAIL] Warning was NOT emitted\n";
            }
        }
    }

    cout << "\n";

    // Test 2: Warnings disabled
    cout << "Test 2: Warnings DISABLED\n";
    cout << "--------------------------\n";
    {
        string code = R"(
            int test() {
                int x = 5;
                float y = 3.14;
                x = y;  // Should NOT warn when disabled
            }
        )";

        Lexer lexer(code, "test.c");
        Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.hasErrors()) {
            SemanticAnalyzer analyzer;
            analyzer.set_warnings_enabled(false);  // Disable warnings
            analyzer.analyze_program(program);

            cout << "Errors: " << analyzer.getErrorHandler().get_error_count() << "\n";
            cout << "Warnings: " << analyzer.getErrorHandler().get_warning_count() << "\n";

            if (analyzer.getErrorHandler().get_warning_count() == 0) {
                cout << "[PASS] Warning was suppressed\n";
            } else {
                cout << "[FAIL] Warning was emitted (should be suppressed)\n";
            }
        }
    }

    cout << "\n";

    // Test 3: Semantic errors (always shown)
    cout << "Test 3: Errors (always shown)\n";
    cout << "-------------------------------\n";
    {
        string code = R"(
            int test() {
                int x = 5;
                y = 10;  // Undeclared identifier error
            }
        )";

        Lexer lexer(code, "test.c");
        Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.hasErrors()) {
            SemanticAnalyzer analyzer;
            analyzer.set_warnings_enabled(false);  // Even with warnings disabled
            analyzer.analyze_program(program);

            cout << "Errors: " << analyzer.getErrorHandler().get_error_count() << "\n";

            if (analyzer.getErrorHandler().get_error_count() > 0) {
                cout << "[PASS] Error was emitted (warnings disabled doesn't affect errors)\n";
            } else {
                cout << "[FAIL] Error was NOT emitted\n";
            }
        }
    }

    cout << "\n";

    // Test 4: Unused variable warnings
    cout << "Test 4: Unused Variable Warnings\n";
    cout << "-----------------------------------\n";
    {
        string code = R"(
            int test() {
                int x = 5;      // Unused variable
                int y = 10;
                return y;       // y is used
            }
        )";

        Lexer lexer(code, "test.c");
        Parser parser(lexer);
        auto program = parser.parseProgram();

        if (!parser.hasErrors()) {
            SemanticAnalyzer analyzer;
            analyzer.set_warnings_enabled(true);  // Warnings enabled
            analyzer.analyze_program(program);

            cout << "Errors: " << analyzer.getErrorHandler().get_error_count() << "\n";
            cout << "Warnings: " << analyzer.getErrorHandler().get_warning_count() << "\n";

            if (analyzer.getErrorHandler().get_warning_count() > 0) {
                cout << "[PASS] Unused variable warning was emitted\n";
            } else {
                cout << "[FAIL] Unused variable warning was NOT emitted\n";
            }
        }
    }

    cout << "\n========================================\n";
    cout << "Temporary Test Complete!\n";
    cout << "========================================\n";

    return 0;
}
