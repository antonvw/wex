////////////////////////////////////////////////////////////////////////////////
// Name:      path-lexer.h
// Purpose:   Declaration of class wex::path_lexer
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/syntax/lexer.h>

namespace wex
{
/// Adds a lexer to a path.
class path_lexer : public path
{
public:
  /// Default constructor using string path.
  explicit path_lexer(const std::string& path = std::string());

  /// Constructor using path.
  explicit path_lexer(const path& p);

  /// Returns true if this path can be built.
  bool is_build() const;

  /// Returns the lexer.
  const auto& lexer() const { return m_lexer; }

private:
  const wex::lexer m_lexer;
};

/// Runs a build on specified file (makefile or ninja build).
/// Returns value from executing the process.
bool build(
  /// the build file
  const path_lexer& p);
}; // namespace wex
