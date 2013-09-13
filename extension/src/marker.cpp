////////////////////////////////////////////////////////////////////////////////
// Name:      marker.cpp
// Purpose:   Implementation of class wxExMarker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/xml/xml.h>
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

wxExMarker::wxExMarker(int no, int symbol)
  : m_No(no)
  , m_Symbol(symbol)
{
}

bool wxExMarker::operator<(const wxExMarker& m) const
{
  return m_No < m.m_No;
}

bool wxExMarker::operator==(const wxExMarker& m) const
{
  return m_No == m.m_No;
}

void wxExMarker::Apply(wxStyledTextCtrl* stc) const
{
  if (IsOk())
  {
    stc->MarkerDefine(
      m_No,
      m_Symbol,
      m_ForegroundColour,
      m_BackgroundColour);
  }
}

bool wxExMarker::IsOk() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_MARKER_MAX &&
    m_Symbol >= 0 && m_Symbol <= wxSTC_MARKER_MAX;
}

void wxExMarker::Set(const wxXmlNode* node)
{
  const wxString single = 
    wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0"));

  if (!single.IsNumber())
  {
    wxLogError("Illegal marker: %s on line: %d", 
      single.c_str(), node->GetLineNumber());
    return;
  }

  m_No = atoi(single.c_str());

  const wxString content = node->GetNodeContent().Strip(wxString::both);

  wxStringTokenizer fields(content, ",");

  const wxString symbol = 
    wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

  m_Symbol = atoi(symbol.c_str());

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

  if (!IsOk())
  {
    wxLogError("Illegal marker number: %d on line: %d", 
      m_No, node->GetLineNumber());
  }
}
