////////////////////////////////////////////////////////////////////////////////
// Name:      marker.cpp
// Purpose:   Implementation of class 'wxExMarker'
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>
#include <wx/extension/marker.h>

wxExMarker::wxExMarker(
    int markerNumber,
    int markerSymbol,
    const wxColour& foreground,
    const wxColour& background)
    : m_MarkerNumber(markerNumber)
    , m_MarkerSymbol(markerSymbol)
    , m_BackgroundColour(background)
    , m_ForegroundColour(foreground)
{
}

void wxExMarker::Apply(wxStyledTextCtrl* stc) const
{
  stc->MarkerDefine(
    m_MarkerNumber,
    m_MarkerSymbol,
    m_ForegroundColour,
    m_BackgroundColour);
}

bool wxExMarker::IsOk() const
{
  return m_MarkerNumber>= 0 && m_MarkerSymbol >= 0;
}
