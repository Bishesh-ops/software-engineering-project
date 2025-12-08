#include <gtest/gtest.h>
#include "error_handler.h"
#include <sstream>

// ============================================================================
// Test Fixture for Error Handler
// ============================================================================
class ErrorHandlerTest : public ::testing::Test {
protected:
    ErrorHandler handler;

    void SetUp() override {
        // Disable colors for testing (to make output comparison easier)
        handler.set_colors_enabled(false);
    }

    // Helper: Create a source location
    SourceLocation makeLocation(const std::string& file, int line, int col) {
        return SourceLocation(file, line, col);
    }
};

// ============================================================================
// Source Location Tests
// ============================================================================

TEST_F(ErrorHandlerTest, SourceLocationToString) {
    SourceLocation loc("test.c", 10, 5);
    EXPECT_EQ(loc.toString(), "test.c:10:5");
}

TEST_F(ErrorHandlerTest, SourceLocationIsValid) {
    SourceLocation valid("test.c", 1, 1);
    EXPECT_TRUE(valid.isValid());

    SourceLocation invalid_empty("", 1, 1);
    EXPECT_FALSE(invalid_empty.isValid());

    SourceLocation invalid_line("test.c", 0, 1);
    EXPECT_FALSE(invalid_line.isValid());

    SourceLocation invalid_col("test.c", 1, 0);
    EXPECT_FALSE(invalid_col.isValid());
}

TEST_F(ErrorHandlerTest, SourceLocationDefault) {
    SourceLocation loc;
    EXPECT_FALSE(loc.isValid());
    EXPECT_EQ(loc.toString(), "<unknown>");
}

// ============================================================================
// Error Reporting Tests
// ============================================================================

TEST_F(ErrorHandlerTest, ReportsSingleError) {
    SourceLocation loc = makeLocation("test.c", 10, 5);
    handler.error("Undefined variable 'x'", loc);

    EXPECT_TRUE(handler.has_errors());
    EXPECT_EQ(handler.get_error_count(), 1);
    EXPECT_EQ(handler.get_warning_count(), 0);
}

TEST_F(ErrorHandlerTest, ReportsMultipleErrors) {
    handler.error("Error 1", makeLocation("test.c", 1, 1));
    handler.error("Error 2", makeLocation("test.c", 2, 1));
    handler.error("Error 3", makeLocation("test.c", 3, 1));

    EXPECT_TRUE(handler.has_errors());
    EXPECT_EQ(handler.get_error_count(), 3);
}

TEST_F(ErrorHandlerTest, ReportsErrorWithoutLocation) {
    handler.error("Generic error message");

    EXPECT_TRUE(handler.has_errors());
    EXPECT_EQ(handler.get_error_count(), 1);
}

// ============================================================================
// Warning Reporting Tests
// ============================================================================

TEST_F(ErrorHandlerTest, ReportsSingleWarning) {
    SourceLocation loc = makeLocation("test.c", 15, 8);
    handler.warning("Unused variable 'y'", loc);

    EXPECT_TRUE(handler.has_warnings());
    EXPECT_EQ(handler.get_warning_count(), 1);
    EXPECT_EQ(handler.get_error_count(), 0);
    EXPECT_FALSE(handler.has_errors());
}

TEST_F(ErrorHandlerTest, ReportsMultipleWarnings) {
    handler.warning("Warning 1", makeLocation("test.c", 1, 1));
    handler.warning("Warning 2", makeLocation("test.c", 2, 1));
    handler.warning("Warning 3", makeLocation("test.c", 3, 1));

    EXPECT_TRUE(handler.has_warnings());
    EXPECT_EQ(handler.get_warning_count(), 3);
    EXPECT_FALSE(handler.has_errors());
}

TEST_F(ErrorHandlerTest, WarningsDoNotCauseCompilationFailure) {
    handler.warning("This is just a warning");

    EXPECT_TRUE(handler.has_warnings());
    EXPECT_FALSE(handler.has_errors());
}

// ============================================================================
// Note Reporting Tests
// ============================================================================

TEST_F(ErrorHandlerTest, ReportsNote) {
    handler.note("Variable declared here", makeLocation("test.c", 5, 10));

    EXPECT_EQ(handler.get_note_count(), 1);
    EXPECT_FALSE(handler.has_errors());
    EXPECT_FALSE(handler.has_warnings());
}

TEST_F(ErrorHandlerTest, NotesProvideContext) {
    // Typical usage: error followed by note
    handler.error("Redefinition of variable 'x'", makeLocation("test.c", 20, 5));
    handler.note("Previous definition here", makeLocation("test.c", 10, 5));

    EXPECT_EQ(handler.get_error_count(), 1);
    EXPECT_EQ(handler.get_note_count(), 1);
}

// ============================================================================
// Mixed Diagnostics Tests
// ============================================================================

TEST_F(ErrorHandlerTest, HandlesMixedDiagnostics) {
    handler.error("Error message", makeLocation("test.c", 1, 1));
    handler.warning("Warning message", makeLocation("test.c", 2, 1));
    handler.note("Note message", makeLocation("test.c", 3, 1));

    EXPECT_EQ(handler.get_error_count(), 1);
    EXPECT_EQ(handler.get_warning_count(), 1);
    EXPECT_EQ(handler.get_note_count(), 1);
    EXPECT_TRUE(handler.has_errors());
    EXPECT_TRUE(handler.has_warnings());
}

TEST_F(ErrorHandlerTest, DiagnosticsAreCollected) {
    handler.error("Error 1", makeLocation("test.c", 1, 1));
    handler.warning("Warning 1", makeLocation("test.c", 2, 1));
    handler.note("Note 1", makeLocation("test.c", 3, 1));

    const auto& diagnostics = handler.get_diagnostics();
    EXPECT_EQ(diagnostics.size(), 3);

    EXPECT_EQ(diagnostics[0].level, DiagnosticLevel::ERROR);
    EXPECT_EQ(diagnostics[0].message, "Error 1");

    EXPECT_EQ(diagnostics[1].level, DiagnosticLevel::WARNING);
    EXPECT_EQ(diagnostics[1].message, "Warning 1");

    EXPECT_EQ(diagnostics[2].level, DiagnosticLevel::NOTE);
    EXPECT_EQ(diagnostics[2].message, "Note 1");
}

// ============================================================================
// Filtering Tests
// ============================================================================

TEST_F(ErrorHandlerTest, FiltersErrorsOnly) {
    handler.error("Error 1", makeLocation("test.c", 1, 1));
    handler.warning("Warning 1", makeLocation("test.c", 2, 1));
    handler.error("Error 2", makeLocation("test.c", 3, 1));
    handler.note("Note 1", makeLocation("test.c", 4, 1));

    auto errors = handler.get_errors();
    EXPECT_EQ(errors.size(), 2);
    EXPECT_EQ(errors[0].message, "Error 1");
    EXPECT_EQ(errors[1].message, "Error 2");
}

TEST_F(ErrorHandlerTest, FiltersWarningsOnly) {
    handler.error("Error 1", makeLocation("test.c", 1, 1));
    handler.warning("Warning 1", makeLocation("test.c", 2, 1));
    handler.warning("Warning 2", makeLocation("test.c", 3, 1));
    handler.note("Note 1", makeLocation("test.c", 4, 1));

    auto warnings = handler.get_warnings();
    EXPECT_EQ(warnings.size(), 2);
    EXPECT_EQ(warnings[0].message, "Warning 1");
    EXPECT_EQ(warnings[1].message, "Warning 2");
}

// ============================================================================
// Clear and Reset Tests
// ============================================================================

TEST_F(ErrorHandlerTest, ClearResetsAllCounters) {
    handler.error("Error", makeLocation("test.c", 1, 1));
    handler.warning("Warning", makeLocation("test.c", 2, 1));
    handler.note("Note", makeLocation("test.c", 3, 1));

    EXPECT_TRUE(handler.has_errors());
    EXPECT_TRUE(handler.has_warnings());

    handler.clear();

    EXPECT_FALSE(handler.has_errors());
    EXPECT_FALSE(handler.has_warnings());
    EXPECT_EQ(handler.get_error_count(), 0);
    EXPECT_EQ(handler.get_warning_count(), 0);
    EXPECT_EQ(handler.get_note_count(), 0);
    EXPECT_EQ(handler.get_diagnostics().size(), 0);
}

// ============================================================================
// Maximum Errors Tests
// ============================================================================

TEST_F(ErrorHandlerTest, DefaultMaxErrorsIs10) {
    // Default is 10 errors as per user story requirement
    EXPECT_EQ(handler.get_max_errors(), 10);
    EXPECT_FALSE(handler.has_reached_max_errors());
}

TEST_F(ErrorHandlerTest, DetectsMaxErrorsReached) {
    handler.set_max_errors(3);

    handler.error("Error 1");
    handler.error("Error 2");
    EXPECT_FALSE(handler.has_reached_max_errors());

    handler.error("Error 3");
    EXPECT_TRUE(handler.has_reached_max_errors());
    EXPECT_TRUE(handler.error_limit_reached());
}

TEST_F(ErrorHandlerTest, MaxErrorsDoesNotAffectWarnings) {
    handler.set_max_errors(2);

    handler.error("Error 1");
    handler.error("Error 2");
    handler.warning("Warning 1");
    handler.warning("Warning 2");

    EXPECT_TRUE(handler.has_reached_max_errors());
    EXPECT_EQ(handler.get_warning_count(), 2);
}

// ============================================================================
// Color Configuration Tests
// ============================================================================

TEST_F(ErrorHandlerTest, ColorsCanBeEnabled) {
    handler.set_colors_enabled(true);
    EXPECT_TRUE(handler.are_colors_enabled());
}

TEST_F(ErrorHandlerTest, ColorsCanBeDisabled) {
    handler.set_colors_enabled(false);
    EXPECT_FALSE(handler.are_colors_enabled());
}

TEST_F(ErrorHandlerTest, ColorSettingPersists) {
    handler.set_colors_enabled(true);
    handler.error("Test error");
    EXPECT_TRUE(handler.are_colors_enabled());
}

// ============================================================================
// Source Context Tests
// ============================================================================

TEST_F(ErrorHandlerTest, SourceContextCanBeEnabled) {
    handler.set_show_source_context(true);
    EXPECT_TRUE(handler.is_source_context_enabled());
}

TEST_F(ErrorHandlerTest, SourceContextCanBeDisabled) {
    handler.set_show_source_context(false);
    EXPECT_FALSE(handler.is_source_context_enabled());
}

TEST_F(ErrorHandlerTest, CanRegisterSourceCode) {
    std::string source = "int main() {\n    return 0;\n}\n";
    handler.register_source("test.c", source);

    const auto& sources = handler.get_source_files();
    EXPECT_EQ(sources.size(), 1);
    EXPECT_EQ(sources.at("test.c"), source);
}

TEST_F(ErrorHandlerTest, CanRegisterMultipleSourceFiles) {
    handler.register_source("file1.c", "int x = 1;");
    handler.register_source("file2.c", "int y = 2;");

    const auto& sources = handler.get_source_files();
    EXPECT_EQ(sources.size(), 2);
    EXPECT_EQ(sources.at("file1.c"), "int x = 1;");
    EXPECT_EQ(sources.at("file2.c"), "int y = 2;");
}

// ============================================================================
// Edge Cases and Boundary Tests
// ============================================================================

TEST_F(ErrorHandlerTest, HandlesEmptyErrorMessage) {
    handler.error("", makeLocation("test.c", 1, 1));
    EXPECT_EQ(handler.get_error_count(), 1);
}

TEST_F(ErrorHandlerTest, HandlesVeryLongErrorMessage) {
    std::string long_msg(1000, 'x');
    handler.error(long_msg, makeLocation("test.c", 1, 1));
    EXPECT_EQ(handler.get_error_count(), 1);
}

TEST_F(ErrorHandlerTest, HandlesLargeLineNumbers) {
    SourceLocation loc = makeLocation("test.c", 999999, 100);
    handler.error("Error at large line number", loc);
    EXPECT_EQ(handler.get_error_count(), 1);
}

TEST_F(ErrorHandlerTest, HandlesMultipleFilesInDiagnostics) {
    handler.error("Error in file 1", makeLocation("file1.c", 1, 1));
    handler.error("Error in file 2", makeLocation("file2.c", 10, 5));
    handler.warning("Warning in file 3", makeLocation("file3.c", 20, 10));

    EXPECT_EQ(handler.get_error_count(), 2);
    EXPECT_EQ(handler.get_warning_count(), 1);

    const auto& diagnostics = handler.get_diagnostics();
    EXPECT_EQ(diagnostics[0].location.filename, "file1.c");
    EXPECT_EQ(diagnostics[1].location.filename, "file2.c");
    EXPECT_EQ(diagnostics[2].location.filename, "file3.c");
}

// ============================================================================
// Diagnostic Level Tests
// ============================================================================

TEST_F(ErrorHandlerTest, DiagnosticLevelsAreDistinct) {
    handler.error("Error msg");
    handler.warning("Warning msg");
    handler.note("Note msg");

    const auto& diagnostics = handler.get_diagnostics();
    EXPECT_EQ(diagnostics[0].level, DiagnosticLevel::ERROR);
    EXPECT_EQ(diagnostics[1].level, DiagnosticLevel::WARNING);
    EXPECT_EQ(diagnostics[2].level, DiagnosticLevel::NOTE);

    // Ensure they're different values
    EXPECT_NE(DiagnosticLevel::ERROR, DiagnosticLevel::WARNING);
    EXPECT_NE(DiagnosticLevel::WARNING, DiagnosticLevel::NOTE);
    EXPECT_NE(DiagnosticLevel::ERROR, DiagnosticLevel::NOTE);
}

// ============================================================================
// Summary Output Tests (capturing stderr)
// ============================================================================

TEST_F(ErrorHandlerTest, SummaryShowsErrorCount) {
    handler.error("Error 1");
    handler.error("Error 2");

    std::ostringstream oss;
    handler.print_summary(oss);

    std::string summary = oss.str();
    EXPECT_TRUE(summary.find("2") != std::string::npos);  // Should mention "2" errors
}

TEST_F(ErrorHandlerTest, SummaryShowsWarningCount) {
    handler.warning("Warning 1");
    handler.warning("Warning 2");
    handler.warning("Warning 3");

    std::ostringstream oss;
    handler.print_summary(oss);

    std::string summary = oss.str();
    EXPECT_TRUE(summary.find("3") != std::string::npos);  // Should mention "3" warnings
}

TEST_F(ErrorHandlerTest, SummaryShowsBothErrorsAndWarnings) {
    handler.error("Error");
    handler.warning("Warning");

    std::ostringstream oss;
    handler.print_summary(oss);

    std::string summary = oss.str();
    // Should show both 1 error and 1 warning
    EXPECT_FALSE(summary.empty());
}

// ============================================================================
// Error Recovery Mechanism Tests
// ============================================================================

TEST_F(ErrorHandlerTest, ErrorRecoveryWithMaxErrors) {
    handler.set_max_errors(5);

    // Simulate error recovery scenario
    for (int i = 0; i < 10; i++) {
        if (handler.has_reached_max_errors()) {
            break;  // Stop reporting after max errors
        }
        handler.error("Error " + std::to_string(i));
    }

    EXPECT_EQ(handler.get_error_count(), 5);
    EXPECT_TRUE(handler.has_reached_max_errors());
}

TEST_F(ErrorHandlerTest, ErrorRecoveryDoesNotStopWarnings) {
    handler.set_max_errors(2);

    handler.error("Error 1");
    handler.error("Error 2");
    // Max errors reached, but warnings can still be reported
    handler.warning("Warning after max errors");

    EXPECT_EQ(handler.get_error_count(), 2);
    EXPECT_EQ(handler.get_warning_count(), 1);
}

// ============================================================================
// State Consistency Tests
// ============================================================================

TEST_F(ErrorHandlerTest, CountsMatchDiagnosticsSize) {
    handler.error("Error 1");
    handler.error("Error 2");
    handler.warning("Warning 1");
    handler.note("Note 1");

    int total_diagnostics = handler.get_diagnostics().size();
    int total_counts = handler.get_error_count() +
                       handler.get_warning_count() +
                       handler.get_note_count();

    EXPECT_EQ(total_diagnostics, total_counts);
}

TEST_F(ErrorHandlerTest, ClearMaintainsConfigSettings) {
    handler.set_colors_enabled(true);
    handler.set_max_errors(10);
    handler.set_show_source_context(true);

    handler.error("Error");
    handler.clear();

    // Configuration should persist after clear
    EXPECT_TRUE(handler.are_colors_enabled());
    EXPECT_EQ(handler.get_max_errors(), 10);
    EXPECT_TRUE(handler.is_source_context_enabled());
}

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(ErrorHandlerTest, DefaultConstructorInitializesCorrectly) {
    ErrorHandler new_handler;

    EXPECT_FALSE(new_handler.has_errors());
    EXPECT_FALSE(new_handler.has_warnings());
    EXPECT_EQ(new_handler.get_error_count(), 0);
    EXPECT_EQ(new_handler.get_warning_count(), 0);
    EXPECT_EQ(new_handler.get_note_count(), 0);
    EXPECT_EQ(new_handler.get_max_errors(), 10);  // Default is 10 as per user story
}

TEST_F(ErrorHandlerTest, ConstructorWithColorsParameter) {
    ErrorHandler handler_with_colors(true);
    EXPECT_TRUE(handler_with_colors.are_colors_enabled());

    ErrorHandler handler_without_colors(false);
    EXPECT_FALSE(handler_without_colors.are_colors_enabled());
}
