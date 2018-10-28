////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.h
// Purpose:   Declaration of class wex::indicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

namespace wex
{
  class stc;

  /// This class defines our scintilla indicators.
  class indicator
  {
  public:
    /// Default constructor.
    indicator(const pugi::xml_node& node = pugi::xml_node());

    /// Constructor.
    /// Only sets no and style, and not the colour and under.
    indicator(int no, int style = -1);

    /// < Operator.
    bool operator<(const indicator& i) const;

    /// == Operator. 
    /// Returns true if no and style are equal
    /// (if style is not -1).
    bool operator==(const indicator& i) const;
    
    /// != Operator.
    bool operator!=(const indicator& i) const {return !operator==(i);};

    /// Applies this indicator to stc component.
    void Apply(stc* stc) const;

    /// Returns foreground colour.
    const auto & GetForegroundColour() const {return m_ForegroundColour;};
    
    /// Returns the no.
    int GetNo() const {return m_No;};

    /// Returns the style.
    int style() const {return m_Style;};
    
    /// Returns underline.
    bool GetUnder() const {return m_Under;};

    /// Returns true if this indicator is valid.
    bool is_ok() const;
  private:
    std::string m_ForegroundColour;
    int m_No = -1, m_Style = -1;
    bool m_Under = false;
  };
};
