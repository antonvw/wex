////////////////////////////////////////////////////////////////////////////////
// Name:      diagnostics.cpp
// Purpose:   Implementation of class wex::lsp::diagnostics
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/diagnostics.h>

namespace wex
{
namespace lsp
{
void diagnostics::add(const std::string& uri, const diagnostic& diag)
{
  m_diagnostics[uri].push_back(diag);
}

void diagnostics::clear(const std::string& uri)
{
  m_diagnostics.erase(uri);
}

void diagnostics::clear_all()
{
  m_diagnostics.clear();
}

const std::vector<diagnostic>& diagnostics::get(const std::string& uri) const
{
  if (const auto it = m_diagnostics.find(uri); it != m_diagnostics.end())
  {
    return it->second;
  }

  static const std::vector<diagnostic> empty;
  return empty;
}

std::vector<diagnostic>
diagnostics::get_line(const std::string& uri, int line) const
{
  std::vector<diagnostic> result;
  const auto&             diags = get(uri);

  for (const auto& diag : diags)
  {
    if (diag.range.start_line == line || diag.range.end_line == line)
    {
      result.push_back(diag);
    }
  }

  return result;
}

std::vector<std::string> diagnostics::get_uris() const
{
  std::vector<std::string> uris;

  for (const auto& pair : m_diagnostics)
  {
    uris.push_back(pair.first);
  }

  return uris;
}

size_t diagnostics::count() const
{
  size_t total = 0;

  for (const auto& pair : m_diagnostics)
  {
    total += pair.second.size();
  }

  return total;
}

bool diagnostics::has(const std::string& uri) const
{
  return m_diagnostics.contains(uri);
}

} // namespace lsp
} // namespace wex
