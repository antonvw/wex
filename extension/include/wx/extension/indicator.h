////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.h
// Purpose:   Declaration of class wxExIndicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXINDICATOR_H
#define _EXINDICATOR_H

#include <wx/colour.h> 

class wxXmlNode;
class wxStyledTextCtrl;

/// This class defines our scintilla indicators.
class WXDLLIMPEXP_BASE wxExIndicator
{
public:
  /// Default constructor.
  wxExIndicator(const wxXmlNode* node = nullptr);

  /// Constructor.
  wxExIndicator(int no, int style);

  /// < operator
  bool operator<(const wxExIndicator& i) const;

  /// == operator
  bool operator==(const wxExIndicator& i) const;

  /// Applies this indicator to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Returns the no.
  int GetNo() const {return m_No;};

  /// Returns true if this indicator is valid.
  bool IsOk() const;
private:
  void Set(const wxXmlNode* node);

  int m_No;
  int m_Style;
  wxColour m_ForegroundColour;
  bool m_Under;
};

#endif
