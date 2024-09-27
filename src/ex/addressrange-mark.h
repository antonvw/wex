////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.h
// Purpose:   Declaration of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/substitute.h>
#include <wex/factory/stc-undo.h>

namespace wex
{
class addressrange;
class block_lines;
class ex;

namespace factory
{
class stc;
}; // namespace factory

/// This class offers a class to handle markers on an addressrange.
/// And if offers the stc_undo.
class addressrange_mark
{
public:
  /// Constructor, specify addressrange, and substitute data.
  addressrange_mark(const addressrange& ar, const data::substitute& subs);

  /// Destructor, removes markers.
  ~addressrange_mark();

  /// Finishes markers, clear indicators if specified.
  void end(bool clear_indicator = true);

  /// Returns block lines for target.
  block_lines get_block_lines() const;

  /// Searches in target for data, updates the target when found.
  bool search();

  /// Sets markers and target, returns false if markers could not be added.
  bool set();

  /// Updates target.
  /// The lines_changed indicates number of lines that was
  /// changed since last update.
  bool update(int lines_changed = 0);

private:
  enum class mark_t
  {
    GLOBAL_APPEND,
    GLOBAL_CHANGE,
    GLOBAL_DELETE,
    GLOBAL_DELETE_INVERSE,
    NORMAL,
  };

  mark_t get_type() const;

  void set_target(int start);

  ex*           m_ex;
  factory::stc* m_stc;

  const addressrange&     m_ar;
  const data::substitute& m_data;

  stc_undo m_undo;

  int  m_corrected{0};
  bool m_last_range_line{false};
};
}; // namespace wex
