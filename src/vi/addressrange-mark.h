////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.h
// Purpose:   Declaration of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class block_lines;
class ex;

namespace factory
{
class stc;
} // namespace factory

/// This class offers a class to handle markers on a addressrange.
class addressrange_mark
{
public:
  /// Constructor, specify addressrange.
  addressrange_mark(const addressrange& ar);

  /// Destructor, removes markers.
  ~addressrange_mark();

  /// Finishes markers, clear indicators if specified.
  void end(bool clear_indicator = true);

  /// Returns block lines for target.
  block_lines get_block_lines() const;

  /// Sets markers and target, returns false if markers could not be added.
  bool set();

  /// Updates target.
  bool update(int begin_pos = -1);

private:
  ex*           m_ex;
  factory::stc* m_stc;

  const addressrange& m_ar;

  int m_corrected{0};
};
}; // namespace wex
