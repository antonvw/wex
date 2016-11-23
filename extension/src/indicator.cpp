////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class wxExIndicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexers.h>

wxExIndicator::wxExIndicator(const wxXmlNode* node)
{
  if (node == nullptr) return;

  const wxString single = 
    wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0").ToStdString());

  if (!single.IsNumber())
  {
    wxLogError("Illegal indicator: %s on line: %d", 
      single.c_str(), node->GetLineNumber());
    return;
  }

  m_No = atoi(single.c_str());

  const wxString content = node->GetNodeContent().Strip(wxString::both);

  wxStringTokenizer fields(content, ",");

  const wxString style = 
    wxExLexers::Get()->ApplyMacro(fields.GetNextToken().ToStdString());

  if (!style.IsNumber())
  {
    wxLogError("Illegal indicator style: %s on line: %d", 
      style.c_str(), node->GetLineNumber());
    return;
  }

  m_Style = atoi(style.c_str());

  if (fields.HasMoreTokens())
  {
    m_ForegroundColour = wxExLexers::Get()->ApplyMacro(
      fields.GetNextToken().Strip(wxString::both).ToStdString());

    if (fields.HasMoreTokens())
    {
      m_Under = (fields.GetNextToken().Strip(wxString::both) == "true");
    }
  }

  if (!IsOk())
  {
    wxLogError("Illegal indicator number: %d on line: %d", 
      m_No, node->GetLineNumber());
  }
}

wxExIndicator::wxExIndicator(int no, int style)
  : m_No(no)
  , m_Style(style)
{
}

bool wxExIndicator::operator<(const wxExIndicator& i) const
{
  return m_No < i.m_No;
}

bool wxExIndicator::operator==(const wxExIndicator& i) const
{
  return m_Style == -1 ?
    m_No == i.m_No:
    m_No == i.m_No && m_Style == i.m_Style;
}

void wxExIndicator::Apply(wxStyledTextCtrl* stc) const
{
  if (IsOk())
  {
    stc->IndicatorSetStyle(m_No, m_Style);

    if (!m_ForegroundColour.empty())
    {
      stc->IndicatorSetForeground(m_No, wxString(m_ForegroundColour));
    }

    stc->IndicatorSetUnder(m_No, m_Under);
  }
}

bool wxExIndicator::IsOk() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_INDIC_MAX &&
    m_Style >= 0 && m_Style <= wxSTC_INDIC_ROUNDBOX;
}
