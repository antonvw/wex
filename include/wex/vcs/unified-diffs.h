////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diffs.h
// Purpose:   Declaration of class unified_diffs
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>

namespace wex
{
class stc;
class unified_diff;

/// Offers a class that collects unified diff invocations to be able
/// to iterate through the differences and show them on stc component.
class unified_diffs
{
public:
  /// Constructor, provide stc.
  unified_diffs(stc* s);

  /// Clears all differences, reset iterator.
  void clear();

  /// Returns distance of the iterator to begin of collection.
  int distance() const;

  /// Goto first diff line on stc.
  bool first();

  /// Inserts a unified diff.
  void insert(const unified_diff* diff);

  /// Goto next diff line on stc. If at end, goes to next stc.
  bool next();

  /// Goto previous diff line on stc. If at begin, goes to previous stc.
  bool prev();

  /// Returns number of differences present.
  int size() const { return m_lines.size(); }

private:
  stc* m_stc{nullptr};

  std::set<int>           m_lines;
  std::set<int>::iterator m_lines_it;
};
}; // namespace wex
