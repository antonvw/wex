////////////////////////////////////////////////////////////////////////////////
// Name:      marker.h
// Purpose:   Declaration of class 'wxExMarker'
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXMARKER_H
#define _EXMARKER_H

#include <wx/colour.h> 
#include <wx/xml/xml.h>

class wxStyledTextCtrl;

/// This class defines our scintilla markers.
class WXDLLIMPEXP_BASE wxExMarker
{
public:
  /// Default constructor.
  wxExMarker(const wxXmlNode* node = NULL);

  /// Constructor.
  wxExMarker(int no);

  /// < operator
  bool operator<(const wxExMarker& m) const;

  /// == operator
  bool operator==(const wxExMarker& m) const;

  /// Applies this marker to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Returns the no.
  int GetNo() const {return m_No;};

  /// Returns true if marker is valid.
  bool IsOk() const;
private:
  void Set(const wxXmlNode* node);

  int m_No;
  int m_Symbol;
  wxColour m_BackgroundColour;
  wxColour m_ForegroundColour;
};

#endif
