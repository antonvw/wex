////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream.h
// Purpose:   Declaration of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <fstream>
#include <map>
#include <string>

#include <wex/text-window.h>

namespace wex
{
  class address;
  class addressrange;
  class file;
  class stc;

  namespace data
  {
    class substitute;
  };

  /// Uses a stream for ex mode processing.
  /// Line numbers are stc line numbers, so start at line 0.
  /// All modifications are done in the temp file, and copied to
  /// the work file upon changing. If you ask for a write,
  /// the work file is copied to the original file.
  class ex_stream : public core::text_window
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

    /// Deletes the range.
    bool erase(const addressrange& range);

    /// Finds line containing text and puts on stc.
    /// The text is interpreted as regex, and search is forward.
    bool find(
      const std::string& text,
      int                find_flags = -1,
      bool               find_next  = true) override;

    /// Returns context lines.
    size_t get_context_lines() const { return m_context_lines; };

    /// Returns current line no
    int get_current_line() const;

    /// Returns number of lines, or LINE_COUNT_UNKNOWN if not yet known.
    int get_line_count() const override;

    /// Returns number of lines.
    int get_line_count_request() override;

    /// Gets specified line, and puts on stc.
    void goto_line(int no) override;

    /// Inserts text at specified address.
    bool insert_text(
      const address&     address,
      const std::string& text,
      loc_t              loc = INSERT_BEFORE);

    /// Returns true if stream is modified;
    bool is_modified() const { return m_is_modified; };

    /// Joins all lines in the range.
    bool join(const addressrange& range);

    /// Sets marker.
    bool marker_add(char marker, int line);

    /// Deletes marker.
    bool marker_delete(char marker);

    /// Returns line for marker.
    int marker_line(char marker) const;

    /// Sets the streams. Puts first line on stc.
    /// This must be called before the other methods.
    void stream(file& f);

    /// Substitutes within the range find by replace.
    bool substitute(const addressrange& range, const data::substitute& data);

    /// Writes working stream to file.
    bool write();

    /// Writes range to file.
    bool write(
      const addressrange& range,
      const std::string&  file,
      bool                append = false);

  private:
    bool copy(file* from, file* to);
    bool get_next_line();
    bool get_previous_line();
    void set_text();

    bool m_is_modified{false};

    const size_t m_buffer_size, m_context_lines, m_current_line_size;

    std::fstream* m_stream{nullptr}; // pointer in m_file to actual stream
    file *        m_file{nullptr}, *m_temp{nullptr}, *m_work{nullptr};

    int m_line_no{LINE_COUNT_UNKNOWN}, m_last_line_no{LINE_COUNT_UNKNOWN};

    std::map<char, int> m_markers;

    char* m_buffer;
    char* m_current_line;

    stc* m_stc;
  };
}; // namespace wex
