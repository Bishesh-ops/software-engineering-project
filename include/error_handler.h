#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>

// ============================================================================
// Source Location - Represents a position in source code
// ============================================================================

struct SourceLocation {
    std::string filename;
    int line;
    int column;

    SourceLocation(const std::string& file = "", int ln = 0, int col = 0)
        : filename(file), line(ln), column(col) {}

    std::string toString() const {
        if (filename.empty()) {
            return "<unknown>";
        }
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
    }

    bool isValid() const {
        return !filename.empty() && line > 0 && column > 0;
    }
};

// ============================================================================
// Diagnostic Message - Represents a single error, warning, or note
// ============================================================================

enum class DiagnosticLevel {
    ERROR,
    WARNING,
    NOTE
};

struct Diagnostic {
    DiagnosticLevel level;
    std::string message;
    SourceLocation location;

    Diagnostic(DiagnosticLevel lvl, const std::string& msg, const SourceLocation& loc)
        : level(lvl), message(msg), location(loc) {}
};

// ============================================================================
// ErrorHandler - Unified error reporting for the entire compiler
// ============================================================================
// 
// User Story: Unified Error Handler
// As a compiler, I want centralized error reporting so that errors are consistent
//
// Features:
// - Centralized error/warning/note reporting
// - Color-coded output (red errors, yellow warnings, blue notes)
// - Error counting and compilation status tracking
// - Source location tracking for all diagnostics
// - Support for multiple errors per compilation phase
//
// Usage:
//   ErrorHandler handler;
//   handler.error("Undefined variable 'x'", location);
//   handler.warning("Unused variable 'y'", location);
//   handler.note("Variable declared here", prev_location);
//   if (handler.has_errors()) {
//       // Compilation failed
//   }
//
// ============================================================================

class ErrorHandler {
public:
    // Constructor
    ErrorHandler(bool enable_colors = true);

    // ========================================================================
    // Primary Diagnostic Methods
    // ========================================================================

    // Report an error (compilation will fail)
    void error(const std::string& message, const SourceLocation& location = SourceLocation());
    
    // Report a warning (compilation continues)
    void warning(const std::string& message, const SourceLocation& location = SourceLocation());
    
    // Report a note (additional context for previous error/warning)
    void note(const std::string& message, const SourceLocation& location = SourceLocation());

    // ========================================================================
    // Status Queries
    // ========================================================================

    // Check if any errors have been reported
    bool has_errors() const { return error_count_ > 0; }
    
    // Check if any warnings have been reported
    bool has_warnings() const { return warning_count_ > 0; }
    
    // Get total error count
    int get_error_count() const { return error_count_; }
    
    // Get total warning count
    int get_warning_count() const { return warning_count_; }
    
    // Get total note count
    int get_note_count() const { return note_count_; }

    // ========================================================================
    // Diagnostic Collection
    // ========================================================================

    // Get all diagnostics (errors, warnings, notes)
    const std::vector<Diagnostic>& get_diagnostics() const { return diagnostics_; }
    
    // Get only errors
    std::vector<Diagnostic> get_errors() const;
    
    // Get only warnings
    std::vector<Diagnostic> get_warnings() const;

    // ========================================================================
    // Configuration
    // ========================================================================

    // Enable/disable color output
    void set_colors_enabled(bool enabled) { colors_enabled_ = enabled; }

    // Check if colors are enabled
    bool are_colors_enabled() const { return colors_enabled_; }

    // Set maximum errors before stopping (0 = unlimited)
    void set_max_errors(int max) { max_errors_ = max; }

    // Get maximum errors setting
    int get_max_errors() const { return max_errors_; }

    // Enable/disable source code context display
    void set_show_source_context(bool enabled) { show_source_context_ = enabled; }

    // Check if source context is enabled
    bool is_source_context_enabled() const { return show_source_context_; }

    // Register source code for a file (for context display)
    void register_source(const std::string& filename, const std::string& source_code);

    // Get registered source files (for sharing between error handlers)
    const std::unordered_map<std::string, std::string>& get_source_files() const { return source_files_; }

    // ========================================================================
    // Utility Methods
    // ========================================================================

    // Clear all diagnostics and reset counters
    void clear();
    
    // Print summary of errors and warnings
    void print_summary(std::ostream& os = std::cerr) const;
    
    // Check if error limit has been reached
    bool error_limit_reached() const {
        return max_errors_ > 0 && error_count_ >= max_errors_;
    }

private:
    // ========================================================================
    // Internal State
    // ========================================================================

    std::vector<Diagnostic> diagnostics_;  // All diagnostics
    int error_count_;                      // Total errors
    int warning_count_;                    // Total warnings
    int note_count_;                       // Total notes
    bool colors_enabled_;                  // Color output enabled
    int max_errors_;                       // Maximum errors (0 = unlimited)
    bool show_source_context_;             // Show source code context
    std::unordered_map<std::string, std::string> source_files_;  // filename -> source code

    // ========================================================================
    // Internal Helper Methods
    // ========================================================================

    // Emit a diagnostic message to stderr
    void emit_diagnostic(const Diagnostic& diag);

    // Get ANSI color code for diagnostic level
    std::string get_color_code(DiagnosticLevel level) const;

    // Get diagnostic level name (ERROR, WARNING, NOTE)
    std::string get_level_name(DiagnosticLevel level) const;

    // Extract a specific line from source code
    std::string get_source_line(const std::string& filename, int line_number) const;

    // Display source code context with caret pointing to error location
    void show_source_context(const SourceLocation& location) const;
    
    // ANSI color codes
    static const std::string COLOR_RESET;
    static const std::string COLOR_RED;
    static const std::string COLOR_YELLOW;
    static const std::string COLOR_BLUE;
    static const std::string COLOR_BOLD;
};

#endif // ERROR_HANDLER_H

