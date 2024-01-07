////////////////////////////////////////////////////////////////////////////////
// Name:      auto-indent.h
// Purpose:   Declaration of class wex::auto_indent
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class stc;

/// Offers a class for auto indentation on wex::stc.
class auto_indent
{
public:
  // Static interface

  /// Returns true if active.
  static bool use();

  /// Sets auto indent on or off.
  static void use(bool on);

  // Other methods

  /// Constructor.
  auto_indent(wex::stc* stc);

  /// Auto indents for char on stc component.
  bool on_char(int c);

private:
  bool determine_indent(int level);
  bool find_indented_line(int start_line);
  void indent(int line, int level);
  bool is_indentable(int c) const;

  stc* m_stc;

  int m_indent{0};

  static inline int m_previous_level{-1};
};
}; // namespace wex
