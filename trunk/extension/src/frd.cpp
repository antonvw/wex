/******************************************************************************\
* File:          frd.cpp
* Purpose:       Implementation of wxExFindReplaceData class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/frd.h>
#include <wx/extension/config.h>

using namespace std;

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
  flags |= wxFR_DOWN *      (m_Config->ReadBool(m_TextSearchDown));
  flags |= wxFR_MATCHCASE * (m_Config->ReadBool(m_TextMatchCase));
  flags |= wxFR_WHOLEWORD * (m_Config->ReadBool(m_TextMatchWholeWord));

  SetFlags(flags);

  // Start with this one, as it is used by SetFindString.
  SetIsRegularExpression(m_Config->ReadBool(m_TextRegEx));
  SetFindString(m_Config->Read(m_TextFindWhat));
  SetReplaceString(m_Config->Read(m_TextReplaceWith));

  // This set determines what fields are placed on the Find Files dialogs
  // as a list of checkboxes.
  m_Info.insert(m_TextMatchWholeWord);
  m_Info.insert(m_TextMatchCase);
  m_Info.insert(m_TextRegEx);
}

wxExFindReplaceData::~wxExFindReplaceData()
{
  m_Config->Write(m_TextFindWhat, GetFindString());
  m_Config->Write(m_TextReplaceWith, GetReplaceString());

  m_Config->Write(m_TextMatchCase, MatchCase());
  m_Config->Write(m_TextMatchWholeWord, MatchWord());
  m_Config->Write(m_TextSearchDown, (GetFlags() & wxFR_DOWN) > 0);
  m_Config->Write(m_TextRegEx, m_IsRegularExpression);
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
