////////////////////////////////////////////////////////////////////////////////
// Name:      scope.h
// Purpose:   Implementation of class wex::scope
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/ex/ctags-entry.h>

#include <bitset>
#include <map>
#include <vector>

namespace wex
{
class stc;

/// This class offers scope functionality for maintaining filters
/// during stc auto completion.
class scope
{
public:
  /// A map of variable name with ctags_entry.
  typedef std::map<std::string, ctags_entry> map_t;

  /// Constructor.
  scope(stc* s);

  /// Returns class name for variable.
  const std::string class_name(const std::string& name) const;

  /// Returns true if iterator is at end of filters.
  bool end() const;

  /// Finds text in scope (from current down), returns true if found.
  /// Sets the iterator if found, or invalidates iterator if not found.
  bool find(const std::string& text);

  /// Returns active filter entry for text, might add empty entry
  /// if text is not yet present.
  ctags_entry& get(const std::string& text);

  /// Inserts entry in the filters map, and sets iterator.
  void insert(const std::string& text, const ctags_entry& ce);

  /// Returns the iterator to the map.
  auto& iter() const { return m_it; }

  /// Synchronizes scope filters with current level
  /// in current position stc.
  void sync();

private:
  /// Check level flags.
  enum
  {
    LEVEL_DOWN = 0, ///< level down flag
    LEVEL_UP   = 1, ///< level up flag
  };

  typedef std::bitset<2> check_t;

  void check_levels(check_t type = check_t().set());

  /// Finds text in scope (from current down), returns iterator.
  map_t::const_iterator iterator(const std::string& text) const;

  stc*                  m_stc;
  std::vector<map_t>    m_filters; // filters at a level
  map_t::const_iterator m_it;
};
}; // namespace wex
