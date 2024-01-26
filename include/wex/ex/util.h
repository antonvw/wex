////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Methods to be used by ex lib
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/defs.h>

#define POST_CLOSE(ID, VETO)                      \
  {                                               \
    wxCloseEvent event(ID);                       \
    event.SetCanVeto(VETO);                       \
    wxPostEvent(wxTheApp->GetTopWindow(), event); \
  };

namespace wex
{
namespace factory
{
class stc;
};

/// Appends a ex line no to text, using ex # format:
/// the text will contain the stc string line number.
void append_line_no(std::string& text, int line);

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
  const std::string& flags = "");

/// Returns a string for specified key.
const std::string k_s(wxKeyCode key);

/// Returns whether there is one letter after.
bool one_letter_after(const std::string& text, const std::string& letter);

/// Returns true if a register is specified after letter in text.
bool register_after(const std::string& text, const std::string& letter);
} // namespace wex
