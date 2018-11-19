////////////////////////////////////////////////////////////////////////////////
// Name:      marker.h
// Purpose:   Declaration of class wex::marker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

namespace wex
{
  class stc;

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
    void apply(stc* stc) const;

    /// Returns background colour.
    const auto & background_colour() const {return m_BackgroundColour;};
    
    /// Returns foreground colour.
    const auto & foreground_colour() const {return m_ForegroundColour;};
    
    /// Returns true if marker is valid.
    bool is_ok() const;

    /// Returns the no.
    int number() const {return m_No;};

    /// Returns symbol no.
    int symbol() const {return m_Symbol;};
  private:
    std::string m_BackgroundColour, m_ForegroundColour;
    int m_No = -1, m_Symbol = -1;
  };
};
