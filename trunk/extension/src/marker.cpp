////////////////////////////////////////////////////////////////////////////////
// Name:      marker.cpp
// Purpose:   Implementation of class 'wxExMarker'
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/extension/marker.h>
#include <wx/extension/lexers.h>

wxExMarker::wxExMarker(const wxXmlNode* node)
  : m_MarkerNumber(-1)
  , m_MarkerSymbol(-1)
{
  if (node != NULL)
  {
    Set(node);
  }
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

void wxExMarker::Set(const wxXmlNode* node)
{
  const wxString number = 
    wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0"));
  const wxString props = node->GetNodeContent().Strip(wxString::both);

  wxStringTokenizer prop_fields(props, ",");

  const wxString symbol = 
    wxExLexers::Get()->ApplyMacro(prop_fields.GetNextToken());

  if (prop_fields.HasMoreTokens())
  {
    m_ForegroundColour = wxExLexers::Get()->ApplyMacro(
      prop_fields.GetNextToken().Strip(wxString::both));

    if (prop_fields.HasMoreTokens())
    {
      m_BackgroundColour = wxExLexers::Get()->ApplyMacro(
        prop_fields.GetNextToken().Strip(wxString::both));
    }
  }

  const int no = atoi(number.c_str());
  const int symno = atoi(symbol.c_str());

  if (no > wxSTC_MARKER_MAX || symno > wxSTC_MARKER_MAX)
  {
    wxLogError(_("Illegal marker number: %d or symbol: %d"), 
      no, 
      symno);
  }
  else
  {
    m_MarkerNumber = no;
    m_MarkerSymbol = symno;
  }
}
