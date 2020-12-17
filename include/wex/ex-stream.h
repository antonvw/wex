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
  class ex_stream
  {
  public:
    /// Constructor.
    ex_stream(wex::stc* stc);

    /// Finds text and puts on stc.
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
    auto line() const { return m_line; };

    /// Gets specified line, and puts on stc.
    void line(int no);

    /// Gets next line, and puts on stc.
    void line_next();

    /// Sets stream. Puts first line on stc.
    void stream(std::fstream& fs);

  private:
    std::string get_line() const;

    std::fstream* m_stream{nullptr};

    int m_line{0};

    stc* m_stc;
  };
}; // namespace wex
