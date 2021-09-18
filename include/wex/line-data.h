////////////////////////////////////////////////////////////////////////////////
// Name:      line-data.h
// Purpose:   Declaration of wex::line_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data.h>

import<functional>;

namespace wex
{
/// Offers user data to be used by ctrl classes (as listctrl, styledtextctrl).
/// First you can set the data using Col, Line, Find etc.,
/// then call inject to perform the action.
/// You can set several items, inject prioritizes the actions.
class line_data
{
public:
  /// Returns column.
  const auto col() const { return m_col; }

  /// Sets column.
  /// Goes to column if col_number > 0
  line_data& col(int col);

  /// Returns line number.
  const auto line() const { return m_line; }

  /// Sets line number.
  /// Goes to the line if > 0, if -1 goes to end of file
  line_data& line(int line, std::function<int(int)> f = nullptr);

  /// Resets members to default state.
  virtual void reset();

private:
  int m_col{NUMBER_NOT_SET}, m_line{NUMBER_NOT_SET};
};
}; // namespace wex

inline wex::line_data& wex::line_data::col(int col)
{
  m_col = col;
  return *this;
}

inline wex::line_data& wex::line_data::line(int line, std::function<int(int)> f)
{
  m_line = (f != nullptr ? f(line) : line);

  return *this;
}

inline void wex::line_data::reset()
{
  m_col  = NUMBER_NOT_SET;
  m_line = NUMBER_NOT_SET;
}
