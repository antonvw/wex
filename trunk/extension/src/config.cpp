/******************************************************************************\
* File:          config.cpp
* Purpose:       Implementation of wxExtension config class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/config.h>
#include <wx/extension/frd.h>

using namespace std;

wxExConfig::wxExConfig(
  const wxString& filename,
  long style)
#ifdef EX_PORTABLE
  : wxFileConfig(
#else
  : wxConfig(
#endif
      wxEmptyString,
      wxEmptyString,
      filename,
      wxEmptyString,
      style)
{
  m_FindReplaceData = new wxExFindReplaceData(this);
}

wxExConfig::~wxExConfig()
{
  delete m_FindReplaceData;

  for (
    map<wxString, wxVariant>::const_iterator it = m_Values.begin();
    it != m_Values.end();
    ++it)
  {
    const wxVariant var = it->second;

    if (var.GetType() == "bool")
      Write(it->first, var.GetBool());
    else if (var.GetType() == "long")
      Write(it->first, var.GetLong());
    else if (var.GetType() == "string")
      Write(it->first, var.GetString());
    else wxFAIL;
  }
}

const wxString wxExConfig::GetKeys() const
{
  wxString text;

  for (
    map<wxString, wxVariant>::const_iterator it = m_Values.begin();
    it != m_Values.end();
    ++it)
  {
    text += it->first + "\t" + it->second.GetString() + "\n";
  }

  return text;
}

void wxExConfig::Toggle(const wxString& key) 
{
  m_Values[key] = !m_Values[key].GetBool();
}
