////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wxExStyle class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
  : m_No(no)
  , m_Value(value)
{
}

void wxExStyle::Apply(wxStyledTextCtrl* stc) const
{
  // E.g.
  // 1,2,3=fore:light steel blue,italic,size:8
  // 1,2,3 are the scintilla_styles, and the rest is spec
  wxStringTokenizer scintilla_styles(m_No, ",");

  // So for each scintilla style set the spec.
  while (scintilla_styles.HasMoreTokens())
  {
    const wxString single = wxExLexers::Get()->ApplyMacro(
      scintilla_styles.GetNextToken());
      stc->StyleSetSpec(atoi(single.c_str()), m_Value);
  }
}

void wxExStyle::Set(const wxXmlNode* node)
{
  wxString content = node->GetNodeContent().Strip(wxString::both);

  std::map<wxString, wxString>::const_iterator it = 
    wxExLexers::Get()->GetMacrosStyle().find(content);

  if (it != wxExLexers::Get()->GetMacrosStyle().end())
  {
    content = it->second;
  }

  m_No = wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0"));
  m_Value = content;
}
