////////////////////////////////////////////////////////////////////////////////
// Name:      style.h
// Purpose:   Declaration of wex::style class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <set>

class wxStyledTextCtrl;

namespace wex
{
/// This class defines our scintilla styles. The no as in xml or in the string
/// can be a single style, or several styles separated by a comma.
/// E.g.
/// 1,2,3=fore:light steel blue,italic,size:8
/// 1,2,3 are the scintilla styles numbers, and the rest is spec
class style
{
public:
  /// Default constructor.
  style() { ; }

  /// Constructor using xml node (sets no from the no attribute).
  style(const pugi::xml_node& node, const std::string& macro)
  {
    set(node, macro);
  }

  /// Constructor using no and value.
  style(
    const std::string& no,
    const std::string& value,
    const std::string& macro = "global")
    : m_value(value)
  {
    set_no(no, macro);
  }

  /// Applies this style to stc component.
  /// If no style is present, stc StyleResetDefault is invoked.
  void apply(wxStyledTextCtrl* stc) const;

  /// Clears style.
  void clear();

  /// Is the default style part of these styles.
  bool contains_default_style() const;

  /// Returns the original define.
  const auto& define() const { return m_define; }

  /// Returns true if this style is valid.
  bool is_ok() const { return !m_no.empty() && !m_value.empty(); }

  /// Returns a single lexer number (the first in the set),
  /// or -1 if no number if present.
  int number() const;

  /// Returns the lexer numbers.
  const auto& numbers() const { return m_no; }

  /// Returns the value.
  const auto& value() const { return m_value; }

private:
  void set(const pugi::xml_node& node, const std::string& macro);
  void set_no(
    const std::string&    no,
    const std::string&    macro,
    const pugi::xml_node& node = pugi::xml_node());

  std::set<int> m_no;

  std::string m_define, m_value;
};
}; // namespace wex
