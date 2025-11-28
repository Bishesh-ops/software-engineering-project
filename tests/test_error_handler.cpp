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
    
    cout << "\n========================================" << endl;
    cout << "All Error Handler Tests Passed!" << endl;
    cout << "========================================" << endl;
    
    return 0;
}

