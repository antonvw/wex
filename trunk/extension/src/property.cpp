////////////////////////////////////////////////////////////////////////////////
// Name:      property.cpp
// Purpose:   Implementation of wxExProperty class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>
#include <wx/extension/property.h>

wxExProperty::wxExProperty(const wxXmlNode* node)
{
  if (node != NULL)
  {
    Set(node);
  }
}

void wxExProperty::Apply(wxStyledTextCtrl* stc) const
{
  wxASSERT(!m_Name.empty());
  stc->SetProperty(m_Name, m_Value);
}

void wxExProperty::ApplyReset(wxStyledTextCtrl* stc) const
{
  stc->SetProperty(m_Name, wxEmptyString);
}

void wxExProperty::Set(const wxXmlNode* node)
{
  m_Name = node->GetAttribute("name", "0");
  m_Value = node->GetNodeContent().Strip(wxString::both);
}
