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
    map<wxString, long>::const_iterator it = m_LongValues.begin();
    it != m_LongValues.end();
    ++it)
  {
    Write(it->first, it->second);
  }

  for (
    map<wxString, wxString>::const_iterator its = m_StringValues.begin();
    its != m_StringValues.end();
    ++its)
  {
    Write(its->first, its->second);
  }

  for (
    map<wxString, bool>::const_iterator itb = m_BoolValues.begin();
    itb != m_BoolValues.end();
    ++itb)
  {
    Write(itb->first, itb->second);
  }
}

long wxExConfig::Get(const wxString& key, long default_value) 
{
  std::map<wxString, long>::const_iterator it = m_LongValues.find(key);

  if (it != m_LongValues.end())
  {
    return it->second;
  }
  else
  {
    const long config_value = Read(key, default_value);
    m_LongValues.insert(std::make_pair(key, config_value));
    return config_value;
  }
}

const wxString wxExConfig::Get(
  const wxString& key, const wxString& default_value, const wxChar field_separator) 
{
  std::map<wxString, wxString>::const_iterator it = m_StringValues.find(key);

  if (it != m_StringValues.end())
  {
    const wxString value = it->second;
    return value.BeforeFirst(field_separator);
   }
  else
  {
    const wxString value = Read(key, default_value);
    m_StringValues.insert(std::make_pair(key, value));
    return value.BeforeFirst(field_separator);
  }
}

bool wxExConfig::GetBool(const wxString& key, bool default_value) 
{
  std::map<wxString, bool>::const_iterator it = m_BoolValues.find(key);

  if (it != m_BoolValues.end())
  {
    return it->second;
  }
  else
  {
    const bool config_value = ReadBool(key, default_value);
    m_BoolValues.insert(std::make_pair(key, config_value));
    return config_value;
  }
}

const wxString wxExConfig::GetBoolKeys() const
{
  wxString text;

  for (
    map<wxString, bool>::const_iterator itb = m_BoolValues.begin();
    itb != m_BoolValues.end();
    ++itb)
  {
    text << itb->first << "\t" << itb->second << "\n";
  }

  return text;
}

const wxString wxExConfig::GetLongKeys() const
{
  wxString text;

  for (
    map<wxString, long>::const_iterator itb = m_LongValues.begin();
    itb != m_LongValues.end();
    ++itb)
  {
    text << itb->first << "\t" << itb->second << "\n";
  }

  return text;
}

const wxString wxExConfig::GetStringKeys() const
{
  wxString text;

  for (
    map<wxString, wxString>::const_iterator itb = m_StringValues.begin();
    itb != m_StringValues.end();
    ++itb)
  {
    text += itb->first + "\t" + itb->second + "\n";
  }

  return text;
}

void wxExConfig::Set(const wxString& key, long value) 
{
  m_LongValues[key] = value;
}

void wxExConfig::Set(const wxString& key, const wxString& value) 
{
  m_StringValues[key] = value;
}

void wxExConfig::SetBool(const wxString& key, bool value) 
{
  m_BoolValues[key] = value;
}

void wxExConfig::Toggle(const wxString& key) 
{
  m_BoolValues[key] = !m_BoolValues[key];
}

void wxExConfig::SetFindReplaceData(
  bool matchword, bool matchcase, bool regularexpression)
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

  m_Config->SetBool(_("Match case"), MatchCase());
  m_Config->SetBool(_("Match whole word"), MatchWord());
  m_Config->SetBool(_("Search down"), (GetFlags() & wxFR_DOWN) > 0);
  m_Config->SetBool(_("Regular expression"), m_IsRegularExpression);
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
