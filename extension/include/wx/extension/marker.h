////////////////////////////////////////////////////////////////////////////////
// Name:      marker.h
// Purpose:   Declaration of class wxExMarker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

class wxXmlNode;
class wxStyledTextCtrl;

/// This class defines our scintilla markers.
class WXDLLIMPEXP_BASE wxExMarker
{
public:
  /// Default constructor.
  wxExMarker(const wxXmlNode* node = nullptr);

  /// Constructor.
  /// Only sets no and symbol, and not the colours.
  wxExMarker(int no, int symbol = -1);

  /// < Operator.
  bool operator<(const wxExMarker& m) const;

  /// == Operator. 
  /// Returns true if no and symbol are equal
  /// (if symbol is not -1).
  bool operator==(const wxExMarker& m) const;

  /// != Operator.
  bool operator!=(const wxExMarker& m) const {return !operator==(m);};

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
