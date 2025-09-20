////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.h
// Purpose:   Declaration of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <wex/data/substitute.h>
#include <wex/factory/stc-undo.h>

namespace wex
{
class addressrange;
class ex;

namespace syntax
{
class stc;
}; // namespace syntax

/// This class offers a class to handle markers on an addressrange.
/// And if offers the stc_undo.
class addressrange_mark
{
public:
  /// Constructor, specify addressrange, substitute data, and global option.
  addressrange_mark(
    const addressrange&     ar,
    const data::substitute& subs,
    bool                    global = false);

  /// Destructor, removes markers.
  ~addressrange_mark();

  /// Finishes markers, clear indicators if specified.
  void end(bool clear_indicator = true);

  /// Return marker begin line.
  int marker_begin() const;

  /// Return marker end line.
  int marker_end() const;

  /// Return marker target line.
  int marker_target() const;

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

  enum marker_t
  {
    BEGIN,
    TARGET,
    END,
  };

  mark_t get_type() const;

  void set_target(int start);

  ex*          m_ex;
  syntax::stc* m_stc;

  const addressrange&    m_ar;
  const data::substitute m_data;

  // markers used: begin, target, end
  const std::array<char, 3> m_markers;

  stc_undo m_undo;

  int  m_corrected{0};
  bool m_last_range_line{false};
};
}; // namespace wex
