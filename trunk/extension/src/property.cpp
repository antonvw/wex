////////////////////////////////////////////////////////////////////////////////
// Name:      property.cpp
// Purpose:   Implementation of wxExProperty class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/stc/stc.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExProperty::wxExProperty(const wxFileName& filename)
  : m_FileName(filename)
{
}

void wxExProperty::Set(wxXmlNode* node);
const std::vector<wxString> wxExProperty::ParseTagProperties(
  const wxXmlNode* node) const
{
  std::vector<wxString> text;

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "property")
    {
      text.push_back(
        child->GetAttribute("name", "0") + "=" +
        child->GetNodeContent().Strip(wxString::both));
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined properties tag: %s on line: %d"),
        child->GetName().c_str(), child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return text;
}
