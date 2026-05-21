////////////////////////////////////////////////////////////////////////////////
// Name:      diagnostics.h
// Purpose:   Declaration of LSP diagnostic types and management
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

namespace wex
{
namespace lsp
{
/// Diagnostic severity levels.
enum class severity_t
{
  error       = 1,
  warning     = 2,
  information = 3,
  hint        = 4
};

/// Represents a single diagnostic (e.g., error, warning).
struct diagnostic
{
  std::string  uri;      ///< Document URI
  int          line{0};
  int          character{0};
  int          end_line{0};
  int          end_character{0};
  severity_t   severity{severity_t::error};
  std::string  code;     ///< Optional error code
  std::string  source;   ///< Source of the diagnostic (e.g., "clang-tidy")
  std::string  message;  ///< Diagnostic message
};

/// Manages LSP diagnostics for documents.
class diagnostics
{
public:
  /// Adds a diagnostic for a document.
  void add_diagnostic(const diagnostic& diag);

  /// Clears all diagnostics for a document.
  void clear_document(const std::string& uri);

  /// Clears all diagnostics.
  void clear_all();

  /// Returns diagnostics for a specific document.
  const std::vector<diagnostic>& get_diagnostics(const std::string& uri) const;

  /// Returns all diagnostics.
  const std::vector<diagnostic>& get_all() const { return m_all_diagnostics; }

  /// Returns diagnostics at a specific line.
  std::vector<diagnostic> get_line_diagnostics(
    const std::string& uri,
    int                line) const;

private:
  std::vector<diagnostic> m_all_diagnostics;
};

} // namespace lsp
} // namespace wex
