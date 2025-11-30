#include "../include/error_handler.h"
#include <iostream>
#include <cassert>

using namespace std;

void test_basic_error_reporting() {
    cout << "Test: Basic Error Reporting" << endl;
    
    ErrorHandler handler;
    SourceLocation loc("test.c", 10, 5);
    
    handler.error("Undefined variable 'x'", loc);
    
    assert(handler.has_errors());
    assert(handler.get_error_count() == 1);
    assert(!handler.has_warnings());
    
    cout << "  [PASS] Basic error reporting works" << endl;
}

void test_warning_reporting() {
    cout << "Test: Warning Reporting" << endl;
    
    ErrorHandler handler;
    SourceLocation loc("test.c", 15, 10);
    
    handler.warning("Unused variable 'y'", loc);
    
    assert(!handler.has_errors());
    assert(handler.has_warnings());
    assert(handler.get_warning_count() == 1);
    
    cout << "  [PASS] Warning reporting works" << endl;
}

void test_note_reporting() {
    cout << "Test: Note Reporting" << endl;
    
    ErrorHandler handler;
    SourceLocation loc1("test.c", 20, 5);
    SourceLocation loc2("test.c", 10, 3);
    
    handler.error("Redeclaration of 'foo'", loc1);
    handler.note("Previous declaration was here", loc2);
    
    assert(handler.has_errors());
    assert(handler.get_error_count() == 1);
    assert(handler.get_note_count() == 1);
    
    cout << "  [PASS] Note reporting works" << endl;
}

void test_multiple_errors() {
    cout << "Test: Multiple Errors" << endl;
    
    ErrorHandler handler;
    
    handler.error("Error 1", SourceLocation("test.c", 1, 1));
    handler.error("Error 2", SourceLocation("test.c", 2, 1));
    handler.error("Error 3", SourceLocation("test.c", 3, 1));
    handler.warning("Warning 1", SourceLocation("test.c", 4, 1));
    
    assert(handler.get_error_count() == 3);
    assert(handler.get_warning_count() == 1);
    assert(handler.get_diagnostics().size() == 4);
    
    cout << "  [PASS] Multiple errors tracked correctly" << endl;
}

void test_clear() {
    cout << "Test: Clear Diagnostics" << endl;
    
    ErrorHandler handler;
    
    handler.error("Error", SourceLocation("test.c", 1, 1));
    handler.warning("Warning", SourceLocation("test.c", 2, 1));
    
    assert(handler.has_errors());
    assert(handler.has_warnings());
    
    handler.clear();
    
    assert(!handler.has_errors());
    assert(!handler.has_warnings());
    assert(handler.get_error_count() == 0);
    assert(handler.get_warning_count() == 0);
    
    cout << "  [PASS] Clear works correctly" << endl;
}

void test_color_toggle() {
    cout << "Test: Color Toggle" << endl;
    
    ErrorHandler handler(true);
    assert(handler.are_colors_enabled());
    
    handler.set_colors_enabled(false);
    assert(!handler.are_colors_enabled());
    
    handler.set_colors_enabled(true);
    assert(handler.are_colors_enabled());
    
    cout << "  [PASS] Color toggle works" << endl;
}

void test_error_limit() {
    cout << "Test: Error Limit" << endl;
    
    ErrorHandler handler;
    handler.set_max_errors(3);
    
    handler.error("Error 1", SourceLocation());
    assert(!handler.error_limit_reached());
    
    handler.error("Error 2", SourceLocation());
    assert(!handler.error_limit_reached());
    
    handler.error("Error 3", SourceLocation());
    assert(handler.error_limit_reached());
    
    cout << "  [PASS] Error limit tracking works" << endl;
}

void test_source_location() {
    cout << "Test: Source Location" << endl;
    
    SourceLocation loc1("main.c", 42, 10);
    assert(loc1.isValid());
    assert(loc1.toString() == "main.c:42:10");
    
    SourceLocation loc2;
    assert(!loc2.isValid());
    assert(loc2.toString() == "<unknown>");
    
    cout << "  [PASS] Source location works" << endl;
}

void test_diagnostic_filtering() {
    cout << "Test: Diagnostic Filtering" << endl;

    ErrorHandler handler;

    handler.error("Error 1", SourceLocation("test.c", 1, 1));
    handler.warning("Warning 1", SourceLocation("test.c", 2, 1));
    handler.error("Error 2", SourceLocation("test.c", 3, 1));
    handler.warning("Warning 2", SourceLocation("test.c", 4, 1));

    auto errors = handler.get_errors();
    auto warnings = handler.get_warnings();

    assert(errors.size() == 2);
    assert(warnings.size() == 2);

    cout << "  [PASS] Diagnostic filtering works" << endl;
}

void test_source_code_registration() {
    cout << "Test: Source Code Registration" << endl;

    ErrorHandler handler;
    string source_code = "int main() {\n    return 0;\n}\n";

    handler.register_source("test.c", source_code);

    auto sources = handler.get_source_files();
    assert(sources.size() == 1);
    assert(sources.count("test.c") == 1);
    assert(sources.at("test.c") == source_code);

    cout << "  [PASS] Source code registration works" << endl;
}

void test_source_context_toggle() {
    cout << "Test: Source Context Toggle" << endl;

    ErrorHandler handler;

    // Should be enabled by default
    assert(handler.is_source_context_enabled());

    handler.set_show_source_context(false);
    assert(!handler.is_source_context_enabled());

    handler.set_show_source_context(true);
    assert(handler.is_source_context_enabled());

    cout << "  [PASS] Source context toggle works" << endl;
}

void test_error_with_source_context() {
    cout << "Test: Error with Source Context" << endl;

    ErrorHandler handler(false);  // Disable colors for predictable output

    // Register multi-line source code
    string source_code =
        "int main() {\n"
        "    int x = 10;\n"
        "    int y = x + z;\n"
        "    return 0;\n"
        "}\n";

    handler.register_source("test.c", source_code);

    // Trigger error on line 3, column 17 (the 'z' variable)
    SourceLocation loc("test.c", 3, 17);

    cout << "  Expected output (error with caret):" << endl;
    cout << "  test.c:3:17: error: undeclared variable 'z'" << endl;
    cout << "      int y = x + z;" << endl;
    cout << "                  ^" << endl;

    // This will output to stderr, but we're just testing it doesn't crash
    handler.error("undeclared variable 'z'", loc);

    assert(handler.has_errors());
    assert(handler.get_error_count() == 1);

    cout << "  [PASS] Error with source context works" << endl;
}

void test_multiple_source_files() {
    cout << "Test: Multiple Source Files" << endl;

    ErrorHandler handler;

    string file1 = "int foo() { return 1; }\n";
    string file2 = "int bar() { return 2; }\n";

    handler.register_source("foo.c", file1);
    handler.register_source("bar.c", file2);

    auto sources = handler.get_source_files();
    assert(sources.size() == 2);
    assert(sources.count("foo.c") == 1);
    assert(sources.count("bar.c") == 1);

    cout << "  [PASS] Multiple source files work" << endl;
}

void test_error_without_source_registration() {
    cout << "Test: Error without Source Registration" << endl;

    ErrorHandler handler;

    // Don't register any source code
    SourceLocation loc("unknown.c", 5, 10);

    // This should work gracefully without source context
    handler.error("Some error", loc);

    assert(handler.has_errors());
    assert(handler.get_error_count() == 1);

    cout << "  [PASS] Error without source registration works gracefully" << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "Error Handler Tests" << endl;
    cout << "========================================" << endl;

    test_basic_error_reporting();
    test_warning_reporting();
    test_note_reporting();
    test_multiple_errors();
    test_clear();
    test_color_toggle();
    test_error_limit();
    test_source_location();
    test_diagnostic_filtering();

    cout << "\n--- Source Context Tests ---" << endl;
    test_source_code_registration();
    test_source_context_toggle();
    test_error_with_source_context();
    test_multiple_source_files();
    test_error_without_source_registration();

    cout << "\n========================================" << endl;
    cout << "All Error Handler Tests Passed!" << endl;
    cout << "========================================" << endl;

    return 0;
}

