////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class 'wxExIndicator'
// Author:    Anton van Wezenbeek
// Created:   2010-02-09
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexers.h>

wxExIndicator::wxExIndicator(const wxXmlNode* node)
  : m_No(-1)
  , m_Style(-1)
  , m_Under(false)
{
  if (node != NULL)
  {
    Set(node);
  }
}

void wxExIndicator::Apply(wxStyledTextCtrl* stc) const
{
  wxASSERT(m_No != -1 && m_Style != -1);

  stc->IndicatorSetStyle(m_No, m_Style);

  if (m_ForegroundColour.IsOk())
  {
    stc->IndicatorSetForeground(m_No, m_ForegroundColour);
  }

  stc->IndicatorSetUnder(m_No, m_Under);
}

bool wxExIndicator::IsOk() const
{
  return m_No >= 0 && m_Style >= 0;
}

void wxExIndicator::Set(const wxXmlNode* node)
{
  const int no = atoi(
    wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0")).c_str());

  m_Style = atoi(node->GetNodeContent().Strip(wxString::both).c_str());

  const wxString content = node->GetNodeContent().Strip(wxString::both);

  wxStringTokenizer fields(content, ",");

  const wxString style = 
    wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

  if (fields.HasMoreTokens())
  {
    m_ForegroundColour = wxExLexers::Get()->ApplyMacro(
      fields.GetNextToken().Strip(wxString::both));

    if (fields.HasMoreTokens())
    {
      m_Under = (fields.GetNextToken().Strip(wxString::both) == "true");
    }
  }

  const int styleno = atoi(style.c_str());

  if (no > wxSTC_INDIC_MAX || styleno > wxSTC_INDIC_ROUNDBOX)
  {
    wxLogError(_("Illegal indicator number: %d or style: %d"), 
      no, 
      styleno);
  }
  else
  {
    m_No = no;
    m_Style = styleno;
  }
}
