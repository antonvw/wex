////////////////////////////////////////////////////////////////////////////////
// Name:      presentation.h
// Purpose:   Declaration of class wex::presentation
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

#include <compare>

class wxStyledTextCtrl;

namespace wex
{
/// This class defines our scintilla presentations.
class presentation
{
public:
  /// The presentation types.
  enum presentation_t
  {
    INDICATOR, ///< presents an indicator
    MARKER,    ///< presents a marker
  };

  /// Constructor from xml node.
  presentation(presentation_t t, const pugi::xml_node& node = pugi::xml_node());

  /// Constructor.
  /// Sets no and style as specified.
  presentation(presentation_t t, int no, int style = -1);

  /// Spaceship operator.
  auto operator<=>(presentation const& rhs) const { return m_no <=> rhs.m_no; }

  /// == Operator.
  bool operator==(const presentation&) const = default;

  /// Applies this presentation to stc component.
  void apply(wxStyledTextCtrl* stc) const;

  /// Returns background colour.
  const auto& background_colour() const { return m_background_colour; }

  /// Returns foreground colour.
  const auto& foreground_colour() const { return m_foreground_colour; }

  /// Returns true if this presentation is valid.
  bool is_ok() const;

  /// Returns underlined.
  bool is_underlined() const { return m_under; }

  /// Returns name of presentation.
  const std::string name() const;

  /// Returns the no.
  int number() const { return m_no; }

  /// Returns the style.
  int style() const { return m_style; }

private:
  std::string m_background_colour, m_foreground_colour;

  int m_no{-1}, m_style{-1};

  bool                 m_under{false};
  const presentation_t m_type;
};
}; // namespace wex
