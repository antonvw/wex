////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream.h
// Purpose:   Declaration of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <fstream>
#include <string>

#include <wex/path.h>
#include <wex/stc-core.h>

namespace wex
{
  class addressrange;
  class file;
  class stc;

  /// Uses a stream for ex mode processing.
  /// Line numbers are stc line numbers, so start at line 0.
  /// All modifications are done in the work file, and copied to
  /// the original file upon writing.
  class ex_stream
  {
  public:
    enum loc_t
    {
      INSERT_BEFORE,
      INSERT_AFTER
    };

    /// Constructor.
    ex_stream(wex::stc* stc);

    /// Destructor.
    ~ex_stream();

    /// Deletes range.
    bool erase(const addressrange& range);

    /// Finds line containing text and puts on stc.
    /// The text is interpreted as regex, and search is forward.
    bool find(const std::string& text);

    /// Returns current line no
    int get_current_line() const;

    /// Returns number of lines, or LINE_COUNT_UNKNOWN if not yet known.
    int get_line_count() const;

    /// Returns number of lines.
    int get_line_count_request();

    /// Gets specified line, and puts on stc.
    void goto_line(int no);

    /// Inserts text at specified line.
    bool
    insert_text(int line, const std::string& text, loc_t loc = INSERT_BEFORE);

    /// Joins all lines in range.
    bool join(const addressrange& range);

    /// Sets the streams. Puts first line on stc.
    /// This must be called before the other methods.
    void stream(file& f);

    /// Substitutes wihtin range find by replace.
    bool substitute(
      const addressrange& range,
      const std::string&  find,
      const std::string&  replace);

  private:
    bool get_next_line();
    void set_context();
    void set_text();

    const int m_context_size, m_line_size;

    std::fstream* m_stream{nullptr};
    file *        m_file, *m_work{nullptr};

    int m_line_no{LINE_COUNT_UNKNOWN}, m_last_line_no{LINE_COUNT_UNKNOWN};

    std::string m_context;
    char*       m_current_line;

    stc* m_stc;
  };
}; // namespace wex