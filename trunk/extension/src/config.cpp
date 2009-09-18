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

wxExFindReplaceData::wxExFindReplaceData(wxExConfig* config)
  : wxFindReplaceData()
  , m_Config(config)
  , m_TextFindWhat(_("Find what"))
  , m_TextMatchCase(_("Match case"))
  , m_TextMatchWholeWord(_("Match whole word"))
  , m_TextRegEx(_("Regular expression"))
  , m_TextReplaceWith(_("Replace with"))
  , m_TextSearchDown(_("Search down"))
{
  int flags = 0;
  flags |= wxFR_DOWN *      (m_Config->GetBool(m_TextSearchDown));
  flags |= wxFR_MATCHCASE * (m_Config->GetBool(m_TextMatchCase));
  flags |= wxFR_WHOLEWORD * (m_Config->GetBool(m_TextMatchWholeWord));

  SetFlags(flags);

  // Start with this one, as it is used by SetFindString.
  SetIsRegularExpression(m_Config->GetBool(m_TextRegEx));
  SetFindString(m_Config->Get(m_TextFindWhat));
  SetReplaceString(m_Config->Get(m_TextReplaceWith));

  // This set determines what fields are placed on the Find Files dialogs
  // as a list of checkboxes.
  m_Info.insert(m_TextMatchWholeWord);
  m_Info.insert(m_TextMatchCase);
  m_Info.insert(m_TextRegEx);
}

wxExFindReplaceData::~wxExFindReplaceData()
{
  m_Config->Set(m_TextFindWhat, GetFindString());
  m_Config->Set(m_TextReplaceWith, GetReplaceString());

  m_Config->Set(m_TextMatchCase, MatchCase());
  m_Config->Set(m_TextMatchWholeWord, MatchWord());
  m_Config->Set(m_TextSearchDown, (GetFlags() & wxFR_DOWN) > 0);
  m_Config->Set(m_TextRegEx, m_IsRegularExpression);
}

void wxExFindReplaceData::CreateAndFill(
  wxWindow* parent,
  wxCheckBox* matchcase,
  int matchcase_id,
  wxCheckBox* matchwholeword,
  int matchwholeword_id,
  wxCheckBox* regex,
  int regex_id) const
{
  matchcase->Create(parent, matchcase_id, m_TextMatchCase);
  matchwholeword->Create(parent, matchwholeword_id, m_TextMatchWholeWord);
  regex->Create(parent, regex_id, m_TextRegEx);

  matchcase->SetValue(MatchCase());
  matchwholeword->SetValue(MatchWord());
  regex->SetValue(IsRegularExpression());
}

void wxExFindReplaceData::FromFindString(wxComboBox* cb)
{
  Update(cb, GetFindString());
}

void wxExFindReplaceData::FromReplaceString(wxComboBox* cb)
{
  Update(cb, GetReplaceString());
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

void wxExFindReplaceData::SetFromCheckBoxes(
  const wxCheckBox* matchword, 
  const wxCheckBox* matchcase, 
  const wxCheckBox* regularexpression)
{
  SetMatchWord(matchword->GetValue());
  SetMatchCase(matchcase->GetValue());
  SetIsRegularExpression(regularexpression->GetValue());
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

void wxExFindReplaceData::Update(wxComboBox* cb, const wxString& value) const
{
  if (!value.empty())
  {
    if (cb->FindString(value) == wxNOT_FOUND)
    {
      cb->Append(value);
    }

    cb->SetValue(value);
  }
}
