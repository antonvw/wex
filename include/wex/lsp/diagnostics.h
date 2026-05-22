////////////////////////////////////////////////////////////////////////////////
// Name:      diagnostics.h
// Purpose:   Declaration of class wex::lsp::diagnostics
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace wex
{
namespace lsp
{
/// Diagnostic severity levels.
enum class severity_t
{
  ERROR   = 1,
  WARNING = 2,
  INFO    = 3,
  HINT    = 4
};

/// Represents a single diagnostic (error, warning, etc.) from the language server.
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

/// Manages diagnostics received from language server.
class diagnostics
{
public:
  /// Adds a diagnostic for a document.
  void add(const std::string& uri, const diagnostic& diag);

  /// Clears all diagnostics for a document.
  void clear(const std::string& uri);

  /// Clears all diagnostics for all documents.
  void clear_all();

  /// Returns all diagnostics for a specific document URI.
  const std::vector<diagnostic>& get(const std::string& uri) const;

  /// Returns all diagnostics on a specific line for a document.
  std::vector<diagnostic> get_line(const std::string& uri, int line) const;

  /// Returns all document URIs with diagnostics.
  std::vector<std::string> get_uris() const;

  /// Returns total count of diagnostics.
  size_t count() const;

  /// Returns whether there are any diagnostics for a document.
  bool has(const std::string& uri) const;

private:
  std::unordered_map<std::string, std::vector<diagnostic>> m_diagnostics;
};

} // namespace lsp
} // namespace wex
