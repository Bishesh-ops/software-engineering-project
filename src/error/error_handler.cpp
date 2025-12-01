#include "../../include/error_handler.h"

// ============================================================================
// ANSI Color Codes (Static Constants)
// ============================================================================

const std::string ErrorHandler::COLOR_RESET  = "\033[0m";
const std::string ErrorHandler::COLOR_RED    = "\033[1;31m";  // Bold Red
const std::string ErrorHandler::COLOR_YELLOW = "\033[1;33m";  // Bold Yellow
const std::string ErrorHandler::COLOR_BLUE   = "\033[1;34m";  // Bold Blue
const std::string ErrorHandler::COLOR_BOLD   = "\033[1m";     // Bold

// ============================================================================
// Constructor
// ============================================================================

ErrorHandler::ErrorHandler(bool enable_colors)
    : error_count_(0),
      warning_count_(0),
      note_count_(0),
      colors_enabled_(enable_colors),
      max_errors_(10),  // Stop after 10 errors (user story requirement)
      show_source_context_(true)  // Enable by default
{
}

// ============================================================================
// Primary Diagnostic Methods
// ============================================================================

void ErrorHandler::error(const std::string& message, const SourceLocation& location) {
    // Check if we've reached the max error limit
    if (max_errors_ > 0 && error_count_ >= max_errors_) {
        // Already at limit, don't add more errors
        return;
    }

    Diagnostic diag(DiagnosticLevel::ERROR, message, location);
    diagnostics_.push_back(diag);
    error_count_++;
    emit_diagnostic(diag);

    // If we just reached the limit, emit a note
    if (max_errors_ > 0 && error_count_ == max_errors_) {
        std::cerr << "\n";
        if (colors_enabled_) {
            std::cerr << COLOR_BOLD;
        }
        std::cerr << "Maximum error limit reached (" << max_errors_ << " errors). ";
        std::cerr << "Stopping compilation.";
        if (colors_enabled_) {
            std::cerr << COLOR_RESET;
        }
        std::cerr << "\n";
    }
}

void ErrorHandler::warning(const std::string& message, const SourceLocation& location) {
    Diagnostic diag(DiagnosticLevel::WARNING, message, location);
    diagnostics_.push_back(diag);
    warning_count_++;
    emit_diagnostic(diag);
}

void ErrorHandler::note(const std::string& message, const SourceLocation& location) {
    Diagnostic diag(DiagnosticLevel::NOTE, message, location);
    diagnostics_.push_back(diag);
    note_count_++;
    emit_diagnostic(diag);
}

// ============================================================================
// Diagnostic Collection
// ============================================================================

std::vector<Diagnostic> ErrorHandler::get_errors() const {
    std::vector<Diagnostic> errors;
    for (const auto& diag : diagnostics_) {
        if (diag.level == DiagnosticLevel::ERROR) {
            errors.push_back(diag);
        }
    }
    return errors;
}

std::vector<Diagnostic> ErrorHandler::get_warnings() const {
    std::vector<Diagnostic> warnings;
    for (const auto& diag : diagnostics_) {
        if (diag.level == DiagnosticLevel::WARNING) {
            warnings.push_back(diag);
        }
    }
    return warnings;
}

// ============================================================================
// Utility Methods
// ============================================================================

void ErrorHandler::clear() {
    diagnostics_.clear();
    error_count_ = 0;
    warning_count_ = 0;
    note_count_ = 0;
}

void ErrorHandler::print_summary(std::ostream& os) const {
    if (error_count_ == 0 && warning_count_ == 0) {
        return;  // No diagnostics to summarize
    }

    os << "\n";

    // Format: "Compilation failed: X errors, Y warnings" or "Compilation succeeded: Y warnings"
    if (error_count_ > 0) {
        if (colors_enabled_) {
            os << COLOR_RED << COLOR_BOLD;
        }
        os << "Compilation failed: ";
        if (colors_enabled_) {
            os << COLOR_RESET << COLOR_RED;
        }
        os << error_count_ << " error" << (error_count_ != 1 ? "s" : "");

        if (warning_count_ > 0) {
            os << ", ";
            if (colors_enabled_) {
                os << COLOR_RESET << COLOR_YELLOW;
            }
            os << warning_count_ << " warning" << (warning_count_ != 1 ? "s" : "");
        }

        if (colors_enabled_) {
            os << COLOR_RESET;
        }
        os << "\n";
    } else if (warning_count_ > 0) {
        // Success but with warnings
        if (colors_enabled_) {
            os << COLOR_YELLOW << COLOR_BOLD;
        }
        os << "Compilation succeeded: ";
        if (colors_enabled_) {
            os << COLOR_RESET << COLOR_YELLOW;
        }
        os << warning_count_ << " warning" << (warning_count_ != 1 ? "s" : "");
        if (colors_enabled_) {
            os << COLOR_RESET;
        }
        os << "\n";
    }
}

// ============================================================================
// Internal Helper Methods
// ============================================================================

void ErrorHandler::emit_diagnostic(const Diagnostic& diag) {
    std::ostream& os = std::cerr;

    // Print location if available
    if (diag.location.isValid()) {
        os << diag.location.toString() << ": ";
    }

    // Print diagnostic level with color
    std::string color = get_color_code(diag.level);
    std::string level_name = get_level_name(diag.level);

    if (colors_enabled_ && !color.empty()) {
        os << color << level_name << COLOR_RESET << ": ";
    } else {
        os << level_name << ": ";
    }

    // Print message
    os << diag.message << "\n";

    // Show source code context if available
    show_source_context(diag.location);
}

std::string ErrorHandler::get_color_code(DiagnosticLevel level) const {
    if (!colors_enabled_) {
        return "";
    }

    switch (level) {
        case DiagnosticLevel::ERROR:
            return COLOR_RED;
        case DiagnosticLevel::WARNING:
            return COLOR_YELLOW;
        case DiagnosticLevel::NOTE:
            return COLOR_BLUE;
        default:
            return "";
    }
}

std::string ErrorHandler::get_level_name(DiagnosticLevel level) const {
    switch (level) {
        case DiagnosticLevel::ERROR:
            return "error";
        case DiagnosticLevel::WARNING:
            return "warning";
        case DiagnosticLevel::NOTE:
            return "note";
        default:
            return "unknown";
    }
}

// ============================================================================
// Source Code Context Support
// ============================================================================

void ErrorHandler::register_source(const std::string& filename, const std::string& source_code) {
    source_files_[filename] = source_code;
}

std::string ErrorHandler::get_source_line(const std::string& filename, int line_number) const {
    // Check if we have source code for this file
    auto it = source_files_.find(filename);
    if (it == source_files_.end()) {
        return "";  // Source not available
    }

    const std::string& source = it->second;

    // Split source into lines and extract the requested line
    int current_line = 1;
    std::string::size_type start = 0;

    while (start < source.length()) {
        // Find end of current line
        std::string::size_type end = source.find('\n', start);
        if (end == std::string::npos) {
            end = source.length();
        }

        // Check if this is the line we want
        if (current_line == line_number) {
            return source.substr(start, end - start);
        }

        // Move to next line
        current_line++;
        start = end + 1;
    }

    return "";  // Line number out of range
}

void ErrorHandler::show_source_context(const SourceLocation& location) const {
    if (!show_source_context_ || !location.isValid()) {
        return;
    }

    std::ostream& os = std::cerr;

    // Get the source line
    std::string line = get_source_line(location.filename, location.line);
    if (line.empty()) {
        return;  // No source available
    }

    // Display the source line with proper indentation
    os << "    " << line << "\n";

    // Display caret pointing to the error column
    // Column is 1-based, so we need column-1 spaces plus 4 for indentation
    if (location.column > 0) {
        std::string caret_line(4 + location.column - 1, ' ');
        caret_line += '^';

        if (colors_enabled_) {
            os << COLOR_RED << caret_line << COLOR_RESET << "\n";
        } else {
            os << caret_line << "\n";
        }
    }
}

