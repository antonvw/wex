////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.h
// Purpose:   Declaration of classes related to Language Server Protocol (LSP)
//            support in wex.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

namespace wex
{
/// Diagnostic severity levels.
enum class severity_t
{
  ERROR   = 1,
  WARNING = 2,
  INFO    = 3,
  HINT    = 4
};

struct completion_item_element
{
  std::string label; // Label of the completion item (e.g., function name, variable name)
  int kind{0}; // Completion item kind (e.g., function, variable, etc.)
  std::string detail; // Additional details about the completion item
  std::string documentation; // Documentation string for the completion item
};

/// Represents a completion item from the language server.
struct completion_item
{
  int line{0}; // Line number where the completion is relevant (0-based)
  int character{0}; // Character position within the line (0-based)

  std::vector<completion_item_element> elements;
};

/// Represents a single diagnostic (error, warning, etc.) from the language
/// server.
struct diagnostic_item
{
  /// Range of the diagnostic
  struct range
  {
    int start_line{0};
    int start_character{0};
    int end_line{0};
    int end_character{0};
  } range;

  /// Severity of the diagnostic
  severity_t severity{severity_t::INFO};

  /// Diagnostic code
  std::string code;

  /// Diagnostic message
  std::string message;

  /// Source of the diagnostic (e.g., "clang", "gcc")
  std::string source;
};

struct hover_item
{
  /// Line number where the hover information is relevant (0-based)
  int line{0};
  /// Character position within the line (0-based)
  int character{0};
  /// Contents of the hover information
  std::string contents;
};

/// Type alias for collections of completions.
typedef completion_item completions_t;

/// Type alias for a collection of diagnostics returned by the language server.
typedef std::vector<diagnostic_item> diagnostics_t;

/// Type alias for a hover item returned by the language server.
typedef struct hover_item hover_t;
} // namespace wex
