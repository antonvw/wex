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

#include <wex/ui/lsp.h>

namespace wex
{
namespace lsp
{
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

  /// Returns total count of diagnostics.
  size_t count() const;

  /// Returns all diagnostics for a specific document URI.
  const std::vector<diagnostic>& get(const std::string& uri) const;

  /// Returns all diagnostics on a specific line for a document.
  std::vector<diagnostic> get_line(const std::string& uri, int line) const;

  /// Returns all document URIs with diagnostics.
  std::vector<std::string> get_uris() const;

  /// Returns whether there are any diagnostics for a document.
  bool has(const std::string& uri) const;

private:
  diastd::unordered_map<std::string, diagnostics_t> m_diagnostics;
};

} // namespace lsp
} // namespace wex
