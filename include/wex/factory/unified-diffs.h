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
namespace factory
{
class stc;
class unified_diff;
}; // namespace factory

/// Offers a class that collects unified diff invocations to be able
/// to iterate through the differences and show them on stc component.
class unified_diffs
{
public:
  /// Constructor, provide stc.
  unified_diffs(factory::stc* s);

  /// Clears all differences, reset iterator.
  void clear();

  /// Goto last diff line on stc.
  bool end();

  /// Goto first diff line on stc.
  bool first();

  /// Inserts a unified diff.
  void insert(const factory::unified_diff* diff);

  /// Goto next diff line on stc. If at end, goes to next stc.
  /// If on first position of stc, goes to first diff line.
  bool next();

  /// Returns position of iterator in the collection.
  /// The first element has number 1.
  int pos() const;

  /// Goto previous diff line on stc. If at begin, goes to previous stc.
  /// If on last position of stc, goes to last diff line.
  bool prev();

  /// Returns number of differences present.
  int size() const { return m_lines.size(); }

  /// Shows status message.
  void status() const;

private:
  factory::stc* m_stc{nullptr};

  std::set<int>           m_lines;
  std::set<int>::iterator m_lines_it;
};
}; // namespace wex
