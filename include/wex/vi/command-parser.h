////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser.h
// Purpose:   Declaration of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/vi/ex.h>

namespace wex
{
/// This class offers the ex command parser.
class command_parser
{
public:
  /// Default constructor, provide the complete ex command (after colon),
  /// e.g. "5p".
  command_parser(ex* ex, const std::string& text = std::string());

  /// The command.
  /// mostly a one letter string like "z" for adjust_window
  /// mostly a one letter string like "p" for print
  auto& command() const { return m_cmd; }

  /// Returns true if parsing was ok.
  bool is_ok() const { return m_is_ok; }

  /// The range.
  auto& range() const { return m_range; }

  /// The text (rest), not the original supplied text.
  /// text, as required by command
  auto& text() const { return m_text; }

private:
  /// Parse the text into the components, and calls
  /// the address or addressrange parsing.
  /// Returns true if parsing was ok.
  bool parse(ex* ex);

  enum class address_t
  {
    NO_ADDR,
    ONE_ADDR,
    TWO_ADDR,
  };

  bool        m_is_ok{false};
  std::string m_cmd, m_range, m_text;
  address_t   m_type;
};
}; // namespace wex
