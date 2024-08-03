////////////////////////////////////////////////////////////////////////////////
// Name:      data/dir.h
// Purpose:   Declaration of class wex::data::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/frd.h>
#include <wex/factory/vcs.h>

#include <bitset>
#include <string>

namespace wex::data
{
/// Offers data to be used by the dir class.
class dir
{
public:
  /// Dir flags.
  enum
  {
    FILES     = 0, ///< include files
    DIRS      = 1, ///< include directories
    RECURSIVE = 2, ///< recursive
    HIDDEN    = 3, ///< include hidden files
  };

  /// A typedef containing dir flags.
  typedef std::bitset<4> type_t;

  /// Returns default type flags.
  /// All flags, except HIDDEN are on.
  static type_t type_t_def() { return type_t().set().set(HIDDEN, false); }

  /// Returns the dir spec.
  const auto& dir_spec() const { return m_dir_spec; }

  /// Sets dir specs.
  dir& dir_spec(const std::string& rhs)
  {
    m_dir_spec = rhs;
    return *this;
  }

  /// Returns the file spec.
  const auto& file_spec() const { return m_file_spec; }

  /// Sets file specs.
  dir& file_spec(const std::string& rhs, bool is_regex = false)
  {
    m_file_spec = rhs;
    m_is_regex  = is_regex;
    return *this;
  }

  /// Returns frd.
  auto* find_replace_data() const { return m_frd; }

  /// Sets frd.
  dir& find_replace_data(factory::find_replace_data* rhs)
  {
    m_frd = rhs;
    return *this;
  }

  /// Returns whether file_spec is a regex.
  bool is_regex() const { return m_is_regex; }

  /// Returns max matches to find, or -1 if no max.
  int max_matches() const { return m_max_matches; }

  /// Sets max matches.
  dir& max_matches(int rhs)
  {
    m_max_matches = rhs;
    return *this;
  }

  /// Returns type.
  auto type() const { return m_flags; }

  /// Sets type.
  dir& type(const type_t& rhs)
  {
    m_flags = rhs;
    return *this;
  }

  /// Returns vcs.
  auto* vcs() const { return m_vcs; }

  /// Sets vcs.
  dir& vcs(factory::vcs* rhs)
  {
    m_vcs = rhs;
    return *this;
  }

private:
  factory::find_replace_data* m_frd{nullptr};
  factory::vcs*               m_vcs{nullptr};

  bool        m_is_regex{false};
  int         m_max_matches{-1};
  std::string m_dir_spec, m_file_spec;
  type_t      m_flags{type_t_def()};
};
}; // namespace wex::data
