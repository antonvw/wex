////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser.h
// Purpose:   Declaration of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/ex/command-parser-data.h>

namespace wex
{
class ex;

/// This class offers the ex command parser.
class command_parser : public command_parser_data
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

  /// Returns true if parsing was ok.
  bool is_ok() const { return m_is_ok; }

  /// The type that is parsed.
  address_t type() const { return m_type; }

private:
  bool parse(parse_t);
  bool parse_other();
  bool parse_selection();

  ex* m_ex;

  address_t m_type{address_t::NO_ADDR};
  bool      m_is_ok{false};
};
}; // namespace wex
