////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser.h
// Purpose:   Declaration of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/vi/ex.h>

namespace wex
{
/// This class offers the ex command parser.
class command_parser
{
public:
  /// The possible ex address types.
  enum class address_t
  {
    NO_ADDR,  ///< not address related
    ONE_ADDR, ///< one address related
    TWO_ADDR, ///< two addresses related
  };

  /// The possible ex parse types.
  enum class parse_t
  {
    PARSE, ///< check and parse text
    CHECK, ///< check only
  };

  /// Constructor.
  command_parser(
    /// the ex component
    ex* ex,
    /// provide the complete ex command (after colon),
    /// e.g. "5p".
    /// - if text not empty parses the text, and sets members
    /// - and if type is PARSE also invokes the address or addressrange
    ///   parser
    const std::string& text = std::string(),
    /// specify whether to parse text only (CHECK)
    /// or continue parsing address or addressrange (PARSE)
    parse_t type = parse_t::PARSE);

  /// The command.
  /// mostly a one letter string like "z" for adjust_window
  /// mostly a one letter string like "p" for print
  auto& command() const { return m_cmd; }

  /// Returns true if parsing was ok.
  bool is_ok() const { return m_is_ok; }

  /// The range.
  auto& range() const { return m_range; }

  /// The text (rest), not the original supplied text.
  auto& text() const { return m_text; }

  /// The type that is parsed.
  auto type() const { return m_type; }

private:
  bool parse(parse_t);
  bool parse_other();
  bool parse_selection();

  bool        m_is_ok{false};
  std::string m_cmd, m_range, m_text;
  address_t   m_type{address_t::NO_ADDR};

  ex* m_ex;
};
}; // namespace wex
