////////////////////////////////////////////////////////////////////////////////
// Name:      marker.h
// Purpose:   Declaration of class wex::marker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

class wxStyledTextCtrl;

namespace wex
{
  /// This class defines our scintilla markers.
  class marker
  {
  public:
    /// Default constructor.
    marker(const pugi::xml_node& node = pugi::xml_node());

    /// Constructor.
    /// Only sets no and symbol, and not the colours.
    marker(int no, int symbol = -1);

    /// < Operator.
    bool operator<(const marker& m) const;

    /// == Operator. 
    /// Returns true if no and symbol are equal
    /// (if symbol is not -1).
    bool operator==(const marker& m) const;

    /// != Operator.
    bool operator!=(const marker& m) const {return !operator==(m);};

    /// Applies this marker to stc component.
    void Apply(wxStyledTextCtrl* stc) const;

    /// Returns background colour.
    const auto & GetBackgroundColour() const {return m_BackgroundColour;};
    
    /// Returns foreground colour.
    const auto & GetForegroundColour() const {return m_ForegroundColour;};
    
    /// Returns the no.
    int GetNo() const {return m_No;};

    /// Returns symbol no.
    int GetSymbol() const {return m_Symbol;};
    
    /// Returns true if marker is valid.
    bool IsOk() const;
  private:
    std::string m_BackgroundColour, m_ForegroundColour;
    int m_No = -1, m_Symbol = -1;
  };
};
