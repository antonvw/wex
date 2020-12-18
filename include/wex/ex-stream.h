////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream.h
// Purpose:   Declaration of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <fstream>
#include <string>

namespace wex
{
  class stc;

  /// Uses a stream for ex mode processing.
  /// Line numbers are stc line numbers, so start at line 0.
  class ex_stream
  {
  public:
    /// Constructor.
    ex_stream(wex::stc* stc);

    /// Finds line containing text and puts on stc.
    bool find(
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
      bool find_next = true);

    /// Returns current line no
    int get_current_line() const;

    /// Returns number of lines, or -1 if not yet known.
    int get_line_count() const;

    /// Gets specified line, and puts on stc.
    void goto_line(int no);

    /// Sets stream. Puts first line on stc.
    void stream(std::fstream& fs);

  private:
    bool get_next_line();
    void set_text();

    std::fstream* m_stream{nullptr};

    int m_line_no{-1}, m_last_line_no{-1};

    std::string m_current_line;

    stc* m_stc;
  };
}; // namespace wex
