/******************************************************************************\
* File:          config.cpp
* Purpose:       Implementation of wxWidgets config extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/config.h>

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
    map<wxString, wxVariant>::const_iterator itb = m_Values.begin();
    itb != m_Values.end();
    ++itb)
  {
    text += itb->first + "\t" + itb->second.GetString() + "\n";
  }

  return text;
}

void wxExConfig::Toggle(const wxString& key) 
{
  m_Values[key] = !m_Values[key].GetBool();
}

void wxExConfig::SetFindReplaceData(
  bool matchword, 
  bool matchcase, 
  bool regularexpression)
{
  m_FindReplaceData->SetMatchWord(matchword);
  m_FindReplaceData->SetMatchCase(matchcase);
  m_FindReplaceData->SetIsRegularExpression(regularexpression);
}

wxExFindReplaceData::wxExFindReplaceData(wxExConfig* config)
  : wxFindReplaceData()
  , m_Config(config)
{
  int flags = 0;
  flags |= wxFR_DOWN *      (m_Config->GetBool(_("Search down")));
  flags |= wxFR_MATCHCASE * (m_Config->GetBool(_("Match case")));
  flags |= wxFR_WHOLEWORD * (m_Config->GetBool(_("Match whole word")));

  SetFlags(flags);

  // Start with this one, as it is used by SetFindString.
  SetIsRegularExpression(m_Config->GetBool(_("Regular expression")));
  SetFindString(m_Config->Get(_("Find what")));
  SetReplaceString(m_Config->Get(_("Replace with")));

  m_Info.insert(_("Match whole word"));
  m_Info.insert(_("Match case"));
  m_Info.insert(_("Regular expression"));
}

wxExFindReplaceData::~wxExFindReplaceData()
{
  m_Config->Set(_("Find what"), GetFindString());
  m_Config->Set(_("Replace with"), GetReplaceString());

  m_Config->Set(_("Match case"), MatchCase());
  m_Config->Set(_("Match whole word"), MatchWord());
  m_Config->Set(_("Search down"), (GetFlags() & wxFR_DOWN) > 0);
  m_Config->Set(_("Regular expression"), m_IsRegularExpression);
}

void wxExFindReplaceData::SetFindString(const wxString& value)
{
  wxFindReplaceData::SetFindString(value);
  m_FindStringNoCase = MatchCase() ? GetFindString(): GetFindString().Upper();

  if (IsRegularExpression())
  {
    int flags = wxRE_DEFAULT;
    if (!MatchCase()) flags |= wxRE_ICASE;
    m_FindRegularExpression.Compile(GetFindString(), flags);
  }
}

void wxExFindReplaceData::SetMatchCase(bool value)
{
  int flags = GetFlags();
  if (value) flags |= wxFR_MATCHCASE;
  else       flags &= ~wxFR_MATCHCASE;
  SetFlags(flags);
}

void wxExFindReplaceData::SetMatchWord(bool value)
{
  int flags = GetFlags();
  if (value) flags |= wxFR_WHOLEWORD;
  else       flags &= ~wxFR_WHOLEWORD;
  SetFlags(flags);
}
