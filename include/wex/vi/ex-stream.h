////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream.h
// Purpose:   Declaration of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/text-window.h>

#include <fstream>
#include <map>
#include <string>

namespace wex
{
class address;
class addressrange;
class ex;
class file;

namespace factory
{
class stc;
}

namespace data
{
class substitute;
};

/// Uses a stream for ex mode processing.
/// Line numbers are stc line numbers, so start at line 0.
/// All modifications are done in the temp file, and copied to
/// the work file upon changing. If you ask for a write,
/// the work file is copied to the original file.
class ex_stream : public factory::text_window
{
public:
  enum loc_t
  {
    INSERT_BEFORE,
    INSERT_AFTER
  };

  /// Constructor.
  explicit ex_stream(wex::ex* ex);

  /// Destructor.
  ~ex_stream() override;

  /// Deletes the range.
  bool erase(const addressrange& range);

  /// Returns context lines.
  size_t get_context_lines() const { return m_context_lines; }

  /// Inserts text at specified address.
  bool insert_text(
    const address&     address,
    const std::string& text,
    loc_t              loc = INSERT_BEFORE);

  /// Returns true if we are in block mode.
  /// Block mode implies that no eols were found when
  /// reading lines with max size.
  bool is_block_mode() const { return m_block_mode; }

  /// Returns true if stream is modified;
  bool is_modified() const { return m_is_modified; }

  /// Joins all lines in the range.
  bool join(const addressrange& range);

  /// Sets marker.
  bool marker_add(char marker, int line);

  /// Deletes marker.
  bool marker_delete(char marker);

  /// Returns line for marker.
  /// Returns LINE_NUMBER_UNKNOWN if marker not known.
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

  /// Yanks range to register, default to yank register.
  bool yank(const addressrange& range, char name = '0');

  /// Virtual methods from text_window.

  bool find(const std::string& text, int find_flags = -1, bool find_next = true)
    override;
  int get_current_line() const override;
  int get_line_count() const override;
  int get_line_count_request() override;

  void goto_line(int no) override;

private:
  bool copy(file* from, file* to);
  void filter_line(int start, int end, std::streampos spos);
  bool get_next_line();
  bool get_previous_line();
  void set_text();

  bool m_block_mode{false}, m_is_modified{false};

  const size_t m_buffer_size, m_context_lines;

  size_t m_current_line_size;

  std::fstream* m_stream{nullptr}; // pointer in m_file to actual stream
  file *        m_file{nullptr}, *m_temp{nullptr}, *m_work{nullptr};

  int
    // this is line or block no, in case no eols are present
    m_line_no{LINE_COUNT_UNKNOWN},
    m_last_line_no{LINE_COUNT_UNKNOWN};

  std::map<char, int> m_markers;

  char* m_buffer;
  char* m_current_line;

  factory::stc* m_stc;
  wex::ex*      m_ex;
};
}; // namespace wex
