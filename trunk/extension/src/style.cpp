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
  wxASSERT(!m_No.empty() && !m_Value.empty());

  for (
    std::vector<int>::const_iterator it = m_No.begin();
    it != m_No.end();
    ++it)
  {
    stc->StyleSetSpec(*it, m_Value);
  }
}

bool wxExStyle::IsDefault() const
{
  for (
    std::vector<int>::const_iterator it = m_No.begin();
    it != m_No.end();
    ++it)
  {
    if (*it == wxSTC_STYLE_DEFAULT)
    {
      return true;
    }
  }

  return false;
}

void wxExStyle::Set(const wxXmlNode* node)
{
  SetNo(wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0")));

  wxString content = node->GetNodeContent().Strip(wxString::both);

  std::map<wxString, wxString>::const_iterator it = 
    wxExLexers::Get()->GetMacrosStyle().find(content);

  if (it != wxExLexers::Get()->GetMacrosStyle().end())
  {
    content = it->second;
  }

  m_Value = content;
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
      m_No.push_back(atoi(single.c_str()));
    }
    else
    {
      wxLogError(_("Illegal style: %s"), single.c_str());
    }
  }
}
