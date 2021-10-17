////////////////////////////////////////////////////////////////////////////////
// Name:      data/dir.h
// Purpose:   Declaration of class wex::data::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/frd.h>

#include <bitset>
#include <string>

namespace wex::data
{
/// Offers data to be used by the dir class.
class dir
{
public:
  /// dir flags.
  enum
  {
    FILES     = 0, // include files
    DIRS      = 1, // include directories
    RECURSIVE = 2, // recursive
    HIDDEN    = 3, // include hidden files
  };

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
  dir& file_spec(const std::string& rhs)
  {
    m_file_spec = rhs;
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

private:
  factory::find_replace_data* m_frd{nullptr};

  int         m_max_matches{-1};
  std::string m_dir_spec, m_file_spec;
  type_t      m_flags{type_t_def()};
};
}; // namespace wex::data
