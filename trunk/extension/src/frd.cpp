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

using namespace std;

wxExFindReplaceData* wxExFindReplaceData::m_Self = NULL;

wxExFindReplaceData::wxExFindReplaceData()
  : wxFindReplaceData()
  , m_Config(wxConfigBase::Get())
  , m_FieldSeparator('\x0B')
  , m_TextFindWhat(_("Find what"))
  , m_TextMatchCase(_("Match case"))
  , m_TextMatchWholeWord(_("Match whole word"))
  , m_TextRegEx(_("Regular expression"))
  , m_TextReplaceWith(_("Replace with"))
  , m_TextSearchDown(_("Search down"))
{
  wxASSERT(m_Config != NULL);

  int flags = 0;
  flags |= wxFR_DOWN *      (m_Config->ReadBool(m_TextSearchDown, true));
  flags |= wxFR_MATCHCASE * (m_Config->ReadBool(m_TextMatchCase, false));
  flags |= wxFR_WHOLEWORD * (m_Config->ReadBool(m_TextMatchWholeWord, false));

  SetFlags(flags);

  // Start with this one, as it is used by SetFindString.
  SetIsRegularExpression(m_Config->ReadBool(m_TextRegEx, false));
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
  m_Config->Write(m_TextRegEx, m_IsRegularExpression);
  m_Config->Write(m_TextSearchDown, (GetFlags() & wxFR_DOWN) > 0);
}

wxExFindReplaceData* wxExFindReplaceData::Get(bool createOnDemand)
{
  if (m_Self == NULL)
  {
    m_Self = new wxExFindReplaceData();
  }

  return m_Self;
}

wxExFindReplaceData* wxExFindReplaceData::Set(wxExFindReplaceData* frd)
{
  wxExFindReplaceData* old = m_Self;
  m_Self = frd;
  return old;
}

void wxExFindReplaceData::SetFindString(const wxString& value)
{
  wxFindReplaceData::SetFindString(value.BeforeFirst(m_FieldSeparator));

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

void wxExFindReplaceData::SetReplaceString(const wxString& value)
{
  wxFindReplaceData::SetReplaceString(value.BeforeFirst(m_FieldSeparator));
}
