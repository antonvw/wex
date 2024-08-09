////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.h
// Purpose:   Declaration of class wex::factory::vcs and vcs_admin
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>

#include <string>

namespace wex
{
namespace factory
{
/// Offers a class that contains vcs factory methods.
class vcs
{
public:
  /// Returns true if dir is to be excluded.
  /// Default returns value of m_is_setup.
  virtual bool is_dir_excluded(const path&) const { return m_is_setup; };

  /// Returns true if file is set to be excluded.
  /// Default returns value of m_is_setup.
  virtual bool is_file_excluded(const path&) const { return m_is_setup; };

  /// Sets up the exclude files or dirs, return true
  /// if anything was added. You should invoke is_setup(true)
  /// if anything was added.
  /// Default returns false.
  virtual bool setup_exclude(
    /// The path to start with.
    const path& dir)
  {
    return m_is_setup;
  };

  /// Destructor.
  virtual ~vcs() = default;

  /// Returns whether setup was ok.
  bool is_setup() const { return m_is_setup; };

protected:
  /// Sets the setup member, and return value.
  bool is_setup(bool rhs)
  {
    m_is_setup = rhs;
    return rhs;
  };

private:
  bool m_is_setup{false};
};

/// Offers several vcs admin support methods.
class vcs_admin
{
public:
  /// Constructor.
  vcs_admin(
    /// the vcs admin dir or file
    const std::string& admin,
    /// the current path
    const path& p);

  /// Returns true if admin component exists for path.
  bool exists() const;

  /// Returns true if toplevel is not empty.
  bool is_toplevel() const;

  /// Return toplevel dir.
  path toplevel() const;

private:
  const std::string m_admin;
  const path        m_path;
};
}; // namespace factory
}; // namespace wex
