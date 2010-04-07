////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wxExStyle class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/extension/style.h>
#include <wx/extension/lexers.h>

wxExStyle::wxExStyle(const wxXmlNode* node)
  : m_No()
  , m_Value()
{
  if (node != NULL)
  {
    Set(node);
  }
}

wxExStyle::wxExStyle(const wxString& no, const wxString& value)
  : m_Value(value)
{
  SetNo(no);
}

void wxExStyle::Apply(wxStyledTextCtrl* stc) const
{
  for (
    std::set<int>::const_iterator it = m_No.begin();
    it != m_No.end();
    ++it)
  {
    stc->StyleSetSpec(*it, m_Value);
  }
}

bool wxExStyle::IsDefault() const
{
  std::set<int>::const_iterator it = m_No.find(wxSTC_STYLE_DEFAULT);
  return (it != m_No.end());
}

bool wxExStyle::IsOk() const
{
  return !m_No.empty() && !m_Value.empty();
}

void wxExStyle::Set(const wxXmlNode* node)
{
  SetNo(wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0")));

  m_Value = node->GetNodeContent().Strip(wxString::both);

  std::map<wxString, wxString>::const_iterator it = 
    wxExLexers::Get()->GetMacrosStyle().find(m_Value);

  if (it != wxExLexers::Get()->GetMacrosStyle().end())
  {
    m_Value = it->second;
  }

  if (!IsOk())
  {
    wxLogError(_("Illegal style on line: %d"), node->GetLineNumber());
  }
}

void wxExStyle::SetNo(const wxString& no)
{
  wxStringTokenizer no_fields(no, ",");

  // Collect each single no in the vector.
  while (no_fields.HasMoreTokens())
  {
    const wxString single = 
      wxExLexers::Get()->ApplyMacro(no_fields.GetNextToken());

    if (single.IsNumber())
    {
      m_No.insert(atoi(single.c_str()));
    }
    else
    {
      wxLogError(_("Illegal style: %s"), single.c_str());
    }
  }
}
