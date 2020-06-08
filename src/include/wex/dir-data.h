////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wex::data::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <string>

namespace wex::data
{
  /// Offers find_files method.
  /// By overriding on_dir and on_file you can take care
  /// of what to do with the result.
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

    /// Returns the dir spec.
    const auto& dir_spec() const { return m_dir_spec; };

    /// Sets dir specs.
    dir& dir_spec(const std::string& rhs);

    /// Returns the file spec.
    const auto& file_spec() const { return m_file_spec; };

    /// Sets file specs.
    dir& file_spec(const std::string& rhs);

    /// Returns type.
    auto type() const { return m_flags; };

    /// Sets type.
    dir& type(type_t rhs);

  private:
    std::string m_dir_spec, m_file_spec;
    type_t      m_flags{type_t().set()};
  };
}; // namespace wex::data
