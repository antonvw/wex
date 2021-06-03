////////////////////////////////////////////////////////////////////////////////
// Name:      text-window.h
// Purpose:   Declaration of class wex::factory::text_window
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define LINE_COUNT_UNKNOWN -1
#define LINE_NUMBER_UNKNOWN -1

namespace wex
{
namespace factory
{
/// Offers a text window.
class text_window
{
public:
  /// Destructor.
  virtual ~text_window() = default;

  /// Pure virtual interface.

  /// Finds next or previous.
  virtual bool find(
    /// text to find
    const std::string& text,
    /// search flags to be used:
    /// - wxSTC_FIND_WHOLEWORD
    /// - wxSTC_FIND_MATCHCASE
    /// - wxSTC_FIND_WORDSTART
    /// - wxSTC_FIND_REGEXP
    /// - wxSTC_FIND_POSIX
    /// - if -1, use flags from find replace data
    int find_flags = -1,
    /// finds next or previous
    bool find_next = true) = 0;

  /// Returns current line no
  virtual int get_current_line() const = 0;

  /// Returns number of lines.
  virtual int get_line_count() const = 0;

  /// Request for number of lines.
  virtual int get_line_count_request() = 0;

  /// Goes to specified line.
  virtual void goto_line(int line) = 0;
};
}; // namespace factory
}; // namespace wex
