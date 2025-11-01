////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser-data.h
// Purpose:   Declaration of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
class ex;

class command_parser_data
{
public:
  /// Default constructor.
  command_parser_data(const std::string& text = std::string());

  /// Returns true if this command can be skipped.
  bool is_global_skip() const;

  /// The command.
  /// mostly a one letter string like "z" for adjust_window
  /// mostly a one letter string like "p" for print
  const std::string& command() const { return m_cmd; }

  /// The range.
  const std::string& range() const { return m_range; }

  /// The text (rest), not the original supplied text.
  const std::string& text() const { return m_text; }

protected:
  std::string m_cmd, m_range, m_text;
};
}; // namespace wex
