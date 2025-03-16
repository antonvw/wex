////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diffs.h
// Purpose:   Declaration of class unified_diffs
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wex/factory/unified-diff.h>

namespace wex
{
namespace factory
{
class stc;
}; // namespace factory

/// Offers a class that collects unified diff invocations to be able
/// to iterate through the differences, show them on the stc component,
/// or checkout the difference.
class unified_diffs
{
public:
  /// Constructor, provide stc.
  unified_diffs(factory::stc* s);

  /// Checks out the difference on specified line.
  /// It reverts the changes on that line, and removes key from
  /// the container, and resets the iterator to begin.
  bool checkout(size_t line);

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
  size_t pos() const;

  /// Goto previous diff line on stc. If at begin, goes to previous stc.
  /// If on last position of stc, goes to last diff line.
  bool prev();

  /// Returns number of differences present.
  size_t size() const { return m_lines.size(); }

  /// Shows status message.
  void status() const;

private:
  factory::stc* m_stc{nullptr};

  std::map<size_t, factory::unified_diff> m_lines;
  std::map<size_t, factory::unified_diff>::const_iterator m_lines_it;
};
}; // namespace wex
