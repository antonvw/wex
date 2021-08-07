////////////////////////////////////////////////////////////////////////////////
// Name:      presentation.h
// Purpose:   Declaration of class wex::presentation
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

class wxStyledTextCtrl;

namespace wex
{
  /// This class defines our scintilla presentations.
  class presentation
  {
  public:
    enum presentation_t
    {
      INDICATOR,
      MARKER,
    };

    /// Constructor.
    presentation(
      presentation_t        t,
      const pugi::xml_node& node = pugi::xml_node());

    /// Constructor.
    /// Only sets no and style, and not the colour and under.
    presentation(presentation_t t, int no, int style = -1);

    /// < Operator.
    bool operator<(const presentation& i) const;

    /// == Operator.
    /// Returns true if no and style are equal
    /// (if style is not -1).
    bool operator==(const presentation& i) const;

    /// != Operator.
    bool operator!=(const presentation& i) const { return !operator==(i); }

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
