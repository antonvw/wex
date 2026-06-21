////////////////////////////////////////////////////////////////////////////////
// Name:      capabilities.h
// Purpose:   Declaration of class wex::lsp::capabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/json.hpp>

namespace wex
{
namespace lsp
{
/// Server capabilities tracking.
class capabilities
{
public:
  /// Returns the client capabilities.
  boost::json::object client() const;

  /// Parses the server capabilites, and fills members.
  bool set(const boost::json::object& obj);

  /// Returns true if server supports completion.
  bool support_completion() const { return m_support_completion; }

  /// Returns true if server supports definition.
  bool support_definition() const { return m_support_definition; }

  /// Returns true if server supports diagnostics.
  bool support_diagnostics() const { return m_support_diagnostics; }

  /// Returns true if server supports hover.
  bool support_hover() const { return m_support_hover; }

private:
  bool m_support_hover{false}, m_support_completion{false},
    m_support_definition{false}, m_support_diagnostics{false};
};

} // namespace lsp
} // namespace wex
