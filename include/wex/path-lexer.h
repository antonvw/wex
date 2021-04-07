////////////////////////////////////////////////////////////////////////////////
// Name:      path-lexer.h
// Purpose:   Declaration of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/lexer.h>
#include <wex/path.h>

namespace wex
{
  /// Adds a lexer to a path.
  class path_lexer : public path
  {
  public:
    /// Default constructor using string path.
    path_lexer(const std::string& path = std::string());

    /// Constructor using path.
    path_lexer(const path& p);

    /// Returns the lexer.
    const auto& lexer() const { return m_lexer; };

  private:
    const wex::lexer m_lexer;
  };
}; // namespace wex
