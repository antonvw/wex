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

exConfig::exConfig(
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
  m_FindReplaceData = new exFindReplaceData(this);
}

exConfig::~exConfig()
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

const wxString exConfig::GetBoolKeys() const
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
  
const wxString exConfig::GetLongKeys() const
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
  
const wxString exConfig::GetStringKeys() const
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
  

void exConfig::SetFindReplaceData(bool matchword, bool matchcase)
{
  m_FindReplaceData->SetMatchWord(matchword);
  m_FindReplaceData->SetMatchCase(matchcase);
}

exFindReplaceData::exFindReplaceData(exConfig* config)
  : wxFindReplaceData()
  , m_Config(config)
{
  int flags = 0;
  flags |= wxFR_DOWN *      (m_Config->GetBool(_("Search down")));
  flags |= wxFR_MATCHCASE * (m_Config->GetBool(_("Match case")));
  flags |= wxFR_WHOLEWORD * (m_Config->GetBool(_("Match whole word")));

  SetFlags(flags);

  SetFindString(m_Config->Get(_("Find what")));
  SetReplaceString(m_Config->Get(_("Replace with")));
}

exFindReplaceData::~exFindReplaceData()
{
  m_Config->Set(_("Find what"), GetFindString());
  m_Config->Set(_("Replace with"), GetReplaceString());

  m_Config->SetBool(_("Match case"), MatchCase());
  m_Config->SetBool(_("Match whole word"), MatchWord());
  m_Config->SetBool(_("Search down"), (GetFlags() & wxFR_DOWN) > 0);
}

bool exFindReplaceData::IsRegExp() const
{
  if (GetFlags() & wxFR_WHOLEWORD)
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool exFindReplaceData::SetFindString(const wxString& value)
{
  wxFindReplaceData::SetFindString(value);
  m_FindStringNoCase = MatchCase() ? GetFindString(): GetFindString().Upper();

  if (IsRegExp())
  {
    int flags = wxRE_DEFAULT;
    if (!MatchCase()) flags |= wxRE_ICASE;
    if (!m_FindRegularExpression.Compile(GetFindString(), flags)) return false;
  }

  return true;
}

void exFindReplaceData::SetMatchCase(bool value)
{
  int flags = GetFlags();
  if (value) flags |= wxFR_MATCHCASE;
  else       flags &= ~wxFR_MATCHCASE;
  SetFlags(flags);
}

void exFindReplaceData::SetMatchWord(bool value)
{
  int flags = GetFlags();
  if (value) flags |= wxFR_WHOLEWORD;
  else       flags &= ~wxFR_WHOLEWORD;
  SetFlags(flags);
}

void exFindReplaceData::Update()
{
  SetFindString(m_Config->Get(_("Find what")));
  SetReplaceString(m_Config->Get(_("Replace with")));

  SetMatchWord(m_Config->GetBool(_("Match whole word")));
  SetMatchCase(m_Config->GetBool(_("Match case")));
}
