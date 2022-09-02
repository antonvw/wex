////////////////////////////////////////////////////////////////////////////////
// Name:      blaming.h
// Purpose:   Implementation of class wex::blaming
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include <wex/vcs/vcs.h>

namespace wex
{
/// This class offers blame related functionality.
class blaming
{
public:
  /// Constructor, provide the data.
  blaming(
    /// stc component (for margin revision id etc).
    factory::stc* stc,
    /// offset
    const std::string& offset);

  /// Executes blaming.
  /// Returns false in case of error.
  bool execute(const path& p);

  /// Returns renamed.
  const auto& renamed() const { return m_renamed; };

  /// Returns revision.
  const auto& revision() const { return m_revision; };

  /// Returns vcs.
  auto& vcs() { return m_vcs; };

private:
  bool error(const std::string& msg = std::string());
  bool exec_git();
  bool exec_svn();

  const std::string m_offset, m_range, m_revision;

  std::string m_renamed;

  path m_path;

  wex::vcs m_vcs;
};
} // namespace wex
