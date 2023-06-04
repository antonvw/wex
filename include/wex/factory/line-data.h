////////////////////////////////////////////////////////////////////////////////
// Name:      line-data.h
// Purpose:   Declaration of wex::line_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/defs.h>

#include <functional>
#include <string>

namespace wex
{
/// Offers user data to be used by factory classes.
class line_data
{
public:
  /// Returns column.
  const auto col() const { return m_col; }

  /// Sets column.
  /// Goes to column if col_number > 0
  line_data& col(int col);

  /// Returns command.
  const auto& command() const { return m_command; }

  /// Sets command.
  /// This is a vi command to execute.
  line_data& command(const std::string& command);

  /// Returns true if command is a ctag command.
  auto is_ctag() const { return m_ctagged; }

  /// Sets ctag.
  line_data& is_ctag(bool rhs);

  /// Returns line number.
  const auto line() const { return m_line; }

  /// Sets line number.
  /// Goes to the line if > 0, if -1 goes to end of file
  line_data& line(int line, std::function<int(int)> f = nullptr);

  /// Resets members to default state.
  virtual void reset();

private:
  bool        m_ctagged{false};
  int         m_col{NUMBER_NOT_SET}, m_line{NUMBER_NOT_SET};
  std::string m_command;
};
}; // namespace wex

// implementation

inline wex::line_data& wex::line_data::col(int col)
{
  m_col = col;
  return *this;
}

inline wex::line_data& wex::line_data::command(const std::string& command)
{
  if (!command.empty())
  {
    m_command = command;
  }

  return *this;
}

inline wex::line_data& wex::line_data::is_ctag(bool rhs)
{
  m_ctagged = rhs;
  return *this;
}

inline wex::line_data& wex::line_data::line(int line, std::function<int(int)> f)
{
  m_line = (f != nullptr ? f(line) : line);

  return *this;
}

inline void wex::line_data::reset()
{
  m_col = NUMBER_NOT_SET;
  m_command.clear();
  m_ctagged = false;
  m_line    = NUMBER_NOT_SET;
}
