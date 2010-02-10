////////////////////////////////////////////////////////////////////////////////
// Name:      marker.h
// Purpose:   Declaration of class 'wxExIndicator'
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXINDICATOR_H
#define _EXINDICATOR_H

#include <wx/xml/xml.h>

class wxStyledTextCtrl;

/// This class defines our indicators.
class wxExIndicator
{
public:
  /// Constructor.
  wxExIndicator(const wxXmlNode* node = NULL);

  /// Applies this indicator to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

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
