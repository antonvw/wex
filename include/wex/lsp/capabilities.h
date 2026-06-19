////////////////////////////////////////////////////////////////////////////////
// Name:      capabilities.h
// Purpose:   Declaration of class wex::lsp::caoabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/json.hpp>
#include <string>

namespace wex
{
namespace lsp
{
/// Server capabilities tracking.
class capabilities
{
public:  
  /// Returns the client capabilities.
  boost::json_object client() const;

  /// Parses the server capabilites, and fills members.
  bool set(const boost::json_object& obj);

private:  
  bool m_hover_support{false}, 
    m_completion_support{false},
    m_definition_support{false},
    m_references_support{false},
    m_rename_support{false},
    m_formatting_support{false},
    m_diagnostic_support{false};
};

} // namespace lsp
} // namespace wex
