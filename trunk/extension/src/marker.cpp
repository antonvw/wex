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
  : m_No(-1)
  , m_Symbol(-1)
{
  if (node != NULL)
  {
    Set(node);
  }
}

void wxExMarker::Apply(wxStyledTextCtrl* stc) const
{
  wxASSERT(IsOk());

  stc->MarkerDefine(
    m_No,
    m_Symbol,
    m_ForegroundColour,
    m_BackgroundColour);
}

bool wxExMarker::IsOk() const
{
  return m_No>= 0 && m_Symbol >= 0;
}

void wxExMarker::Set(const wxXmlNode* node)
{
  const int no = 
    atoi(wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0")).c_str());

  const wxString content = node->GetNodeContent().Strip(wxString::both);

  wxStringTokenizer fields(content, ",");

  const wxString symbol = 
    wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

  if (fields.HasMoreTokens())
  {
    m_ForegroundColour = wxExLexers::Get()->ApplyMacro(
      fields.GetNextToken().Strip(wxString::both));

    if (fields.HasMoreTokens())
    {
      m_BackgroundColour = wxExLexers::Get()->ApplyMacro(
        fields.GetNextToken().Strip(wxString::both));
    }
  }

  const int symno = atoi(symbol.c_str());

  if (no > wxSTC_MARKER_MAX || symno > wxSTC_MARKER_MAX)
  {
    wxLogError(_("Illegal marker number: %d or symbol: %d"), 
      no, 
      symno);
  }
  else
  {
    m_No = no;
    m_Symbol = symno;
  }
}
