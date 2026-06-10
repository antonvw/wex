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

/// Represents a single diagnostic (error, warning, etc.) from the language
/// server.
struct diagnostic
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

struct hover
{
  int line{
    0}; ///< Line number where the hover information is relevant (0-based)
  int character{0}; ///< Character position within the line (0-based)
  /// Contents of the hover information
  std::string contents;
};

typedef struct hover            hover_t;
typedef std::vector<diagnostic> diagnostics_t;
} // namespace wex
