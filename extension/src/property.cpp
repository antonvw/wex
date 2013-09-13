////////////////////////////////////////////////////////////////////////////////
// Name:      property.cpp
// Purpose:   Implementation of wxExProperty class
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
#include <wx/extension/property.h>

wxExProperty::wxExProperty(const wxXmlNode* node)
{
  if (node != NULL)
  {
    Set(node);
  }
}

wxExProperty::wxExProperty(const wxString& name, const wxString& value)
  : m_Name(name)
  , m_Value(value)
{
}
  
void wxExProperty::Apply(wxStyledTextCtrl* stc) const
{
  if (IsOk())
  {
    stc->SetProperty(m_Name, m_Value);
  }
}

void wxExProperty::ApplyReset(wxStyledTextCtrl* stc) const
{
  stc->SetProperty(m_Name, wxEmptyString);
}

bool wxExProperty::IsOk() const
{
  return !m_Name.empty() && !m_Value.empty();
}

void wxExProperty::Set(const wxXmlNode* node)
{
  m_Name = node->GetAttribute("name", "0");
  m_Value = node->GetNodeContent().Strip(wxString::both);

  if (!IsOk())
  {
    wxLogError("Illegal property name: %s or value: %s on line: %d",
      m_Name.c_str(),
      m_Value.c_str(),
      node->GetLineNumber());
  }
}
