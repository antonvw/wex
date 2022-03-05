////////////////////////////////////////////////////////////////////////////////
// Name:      vi/util.h
// Purpose:   Methods to be used by vi lib
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define REPEAT(TEXT)                   \
  {                                    \
    for (auto i = 0; i < m_count; i++) \
    {                                  \
      TEXT;                            \
    }                                  \
  }

#define REPEAT_WITH_UNDO(TEXT) \
  {                            \
    stc_undo undo(get_stc());  \
    REPEAT(TEXT);              \
  }

namespace wex
{
/// Returns escape sequence.
const std::string esc();

/// Returns substring after first occurrence of one of specified chars.
const std::string find_first_of(
  /// text to be searched
  const std::string& text,
  /// another string with the characters to search for
  const std::string& chars,
  /// position of the first character in the string to be considered in the
  /// search
  const size_t pos = 0);

/// Returns string with lines from stc component.
std::string get_lines(
  /// stc component
  factory::stc* stc,
  /// start line
  int start,
  /// end line
  int end,
  /// flags to specify behaviour:
  /// - If the # flag is specified or the number edit option is set,
  ///   each line shall be preceded by its line number in the following format:
  ///   "%6d", <line number>
  /// - If the l flag is specified or the list edit option is set:
  ///   The end of each line shall be marked with a '$',
  ///   and literal '$' characters within the line shall be written with a
  ///   preceding <backslash>.
  const std::string& flags);

/// Returns a string for specified key.
const std::string k_s(wxKeyCode key);

/// Returns whether there is one letter after.
bool one_letter_after(const std::string& text, const std::string& letter);

/// Returns true if a register is specified after letter in text.
bool register_after(const std::string& text, const std::string& letter);
} // namespace wex
