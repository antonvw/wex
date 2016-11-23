////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.h
// Purpose:   Declaration of class wxExIndicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

class wxXmlNode;
class wxStyledTextCtrl;

/// This class defines our scintilla indicators.
class WXDLLIMPEXP_BASE wxExIndicator
{
public:
  /// Default constructor.
  wxExIndicator(const wxXmlNode* node = nullptr);

  /// Constructor.
  /// Only sets no and style, and not the colour and under.
  wxExIndicator(int no, int style = -1);

  /// < Operator.
  bool operator<(const wxExIndicator& i) const;

  /// == Operator. 
  /// Returns true if no and style are equal
  /// (if style is not -1).
  bool operator==(const wxExIndicator& i) const;
  
  /// != Operator.
  bool operator!=(const wxExIndicator& i) const {return !operator==(i);};

  /// Applies this indicator to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Returns foreground colour.
  const auto & GetForegroundColour() const {return m_ForegroundColour;};
  
  /// Returns the no.
  int GetNo() const {return m_No;};

  /// Returns the style.
  int GetStyle() const {return m_Style;};
  
  /// Returns underline.
  bool GetUnder() const {return m_Under;};

  /// Returns true if this indicator is valid.
  bool IsOk() const;
private:
  std::string m_ForegroundColour;
  int m_No = -1, m_Style = -1;
  bool m_Under = false;
};
