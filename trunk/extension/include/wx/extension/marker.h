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

/// This class defines our markers, closely related to scintilla markers.
class wxExMarker
{
public:
  /// Constructor.
  wxExMarker(
    int markerNumber,
    int markerSymbol,
    const wxColour& foreground = wxNullColour,
    const wxColour& background = wxNullColour);

  /// Applies this marker to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Returns true if marker is valid.
  bool IsOk() const;
private:
  // When making const, Visual Studio complains:
  // error C2582: 'operator =' function is unavailable in 'wxExMarker'
  int m_MarkerNumber;
  int m_MarkerSymbol;
  wxColour m_BackgroundColour;
  wxColour m_ForegroundColour;
};

#endif
