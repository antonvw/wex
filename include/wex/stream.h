////////////////////////////////////////////////////////////////////////////////
// Name:      stream.h
// Purpose:   Declaration of wex::stream class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/path-lexer.h>
#include <wex/path-match.h>
#include <wex/stream-statistics.h>
#include <wex/tool.h>

class wxEvtHandler;

namespace wex
{
namespace factory
{
class find_replace_data;
};

/// Adds run_tool methods and statistics to a file stream.
class stream
{
public:
  /// Constructor.
  stream(
    wex::factory::find_replace_data* frd,
    const wex::path&                 path,
    const tool&                      tool,
    wxEvtHandler*                    eh = nullptr);

  /// Returns the statistics.
  const auto& get_statistics() const { return m_stats; }

  /// Returns the tool.
  const auto& get_tool() const { return m_tool; }

  /// Returns the filename.
  const auto& path() const { return m_path; }

  /// Runs the tool.
  bool run_tool();

private:
  auto inc_actions_completed(int inc_value = 1)
  {
    return m_stats.m_elements.inc(
      _("Actions Completed").ToStdString(),
      inc_value);
  }

  auto inc_statistics(const std::string& keyword)
  {
    return m_stats.m_elements.inc(keyword);
  }

  bool is_word_character(int c) const { return isalnum(c) || c == '_'; }

  bool process(std::string& text, size_t line_no);
  bool process_begin();
  void process_match(const path_match& m);

  int replace_all(std::string& text, int* match_pos);

  const path_lexer m_path;
  const tool       m_tool;
  const int        m_threshold;

  stream_statistics m_stats;
  int               m_prev{0};

  bool m_modified{false}, m_write{false};

  wxEvtHandler* m_eh{nullptr};

  wex::factory::find_replace_data* m_frd;
  std::string                      m_find_string;
  static inline bool               m_asked{false};
};
}; // namespace wex
