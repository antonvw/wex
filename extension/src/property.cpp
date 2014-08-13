////////////////////////////////////////////////////////////////////////////////
// Name:      property.cpp
// Purpose:   Implementation of wxExProperty class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/xml/xml.h>
#include <wx/extension/property.h>

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
