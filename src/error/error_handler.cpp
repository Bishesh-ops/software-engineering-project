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
      max_errors_(0)  // 0 = unlimited
{
}

// ============================================================================
// Primary Diagnostic Methods
// ============================================================================

void ErrorHandler::error(const std::string& message, const SourceLocation& location) {
    Diagnostic diag(DiagnosticLevel::ERROR, message, location);
    diagnostics_.push_back(diag);
    error_count_++;
    emit_diagnostic(diag);
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
    os << "========================================\n";
    os << "Compilation Summary\n";
    os << "========================================\n";

    if (error_count_ > 0) {
        if (colors_enabled_) {
            os << COLOR_RED;
        }
        os << error_count_ << " error" << (error_count_ != 1 ? "s" : "");
        if (colors_enabled_) {
            os << COLOR_RESET;
        }
        os << " generated\n";
    }

    if (warning_count_ > 0) {
        if (colors_enabled_) {
            os << COLOR_YELLOW;
        }
        os << warning_count_ << " warning" << (warning_count_ != 1 ? "s" : "");
        if (colors_enabled_) {
            os << COLOR_RESET;
        }
        os << " generated\n";
    }

    os << "========================================\n";
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

