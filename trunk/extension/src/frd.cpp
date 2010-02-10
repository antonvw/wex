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

#include <wx/config.h> 
#include <wx/checklst.h> 
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

wxExFindReplaceData* wxExFindReplaceData::m_Self = NULL;

wxExFindReplaceData::wxExFindReplaceData()
  : wxFindReplaceData()
  , m_TextFindWhat(_("Find what"))
  , m_TextMatchCase(_("Match case"))
  , m_TextMatchWholeWord(_("Match whole word"))
  , m_TextRegEx(_("Regular expression"))
  , m_TextReplaceWith(_("Replace with"))
  , m_TextSearchDown(_("Search down"))
{
  int flags = 0;
  flags |= wxFR_DOWN *      (wxConfigBase::Get()->ReadBool(m_TextSearchDown, true));
  flags |= wxFR_MATCHCASE * (wxConfigBase::Get()->ReadBool(m_TextMatchCase, false));
  flags |= wxFR_WHOLEWORD * (wxConfigBase::Get()->ReadBool(m_TextMatchWholeWord, false));

  SetFlags(flags);

  // Start with this one, as it is used by SetFindString.
  SetIsRegularExpression(wxConfigBase::Get()->ReadBool(m_TextRegEx, false));

  m_FindStrings = wxExListFromConfig(m_TextFindWhat);
  m_ReplaceStrings = wxExListFromConfig(m_TextReplaceWith);

  if (!m_FindStrings.empty())
  {
    wxFindReplaceData::SetFindString(m_FindStrings.front());
  }

  if (!m_ReplaceStrings.empty())
  {
    wxFindReplaceData::SetReplaceString(m_ReplaceStrings.front());
  }

  // This set determines what fields are placed on the Find Files dialogs
  // as a list of checkboxes.
  m_Info.insert(m_TextMatchWholeWord);
  m_Info.insert(m_TextMatchCase);
  m_Info.insert(m_TextRegEx);
}

wxExFindReplaceData::~wxExFindReplaceData()
{
  wxExListToConfig(m_FindStrings, m_TextFindWhat);
  wxExListToConfig(m_ReplaceStrings, m_TextReplaceWith);

  wxConfigBase::Get()->Write(m_TextMatchCase, MatchCase());
  wxConfigBase::Get()->Write(m_TextMatchWholeWord, MatchWord());
  wxConfigBase::Get()->Write(m_TextRegEx, m_IsRegularExpression);
  wxConfigBase::Get()->Write(m_TextSearchDown, SearchDown());
}

wxExFindReplaceData* wxExFindReplaceData::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExFindReplaceData();
  }

  return m_Self;
}

bool wxExFindReplaceData::Get(
  const wxString& field, 
  wxCheckListBox* clb, 
  int item) const
{
  if (field == m_TextMatchWholeWord)
  {
    clb->Check(item, MatchWord());
  }
  else if (field == m_TextMatchCase)
  {
    clb->Check(item, MatchCase());
  }
  else if (field == m_TextRegEx)
  {
    clb->Check(item, IsRegularExpression());
  }
  else
  {
    return false;
  }

  return true;
}

const wxString wxExFindReplaceData::GetFindReplaceInfoText(bool replace) const
{
  // TODO: wx 2.9.1 GetFindString is no const, so use a cast here.

  wxString log = _("Searching for") + ": " + 
    const_cast< wxExFindReplaceData * >( this )->GetFindString();

  if (replace)
  {
    log += " " + _("Replacing with") + ": " + 
      const_cast< wxExFindReplaceData * >( this )->GetReplaceString();
  }

  return log;
}

wxExFindReplaceData* wxExFindReplaceData::Set(wxExFindReplaceData* frd)
{
  wxExFindReplaceData* old = m_Self;
  m_Self = frd;
  return old;
}

bool wxExFindReplaceData::Set(const wxString& field, bool value)
{
  if (field == m_TextMatchWholeWord)
  {
    SetMatchWord(value);
  }
  else if (field == m_TextMatchCase)
  {
    SetMatchCase(value);
  }
  else if (field == m_TextRegEx)
  {
    SetIsRegularExpression(value);
  }
  else
  {
    return false;
  }

  return true;
}

void wxExFindReplaceData::SetFindRegularExpression()
{
  if (IsRegularExpression())
  {
    int flags = wxRE_DEFAULT;
    if (!MatchCase()) flags |= wxRE_ICASE;
    m_FindRegularExpression.Compile(GetFindString(), flags);
  }
}

void wxExFindReplaceData::SetFindString(const wxString& value)
{
  wxFindReplaceData::SetFindString(value);

  m_FindStrings.remove(value);
  m_FindStrings.push_front(value);

  SetFindRegularExpression();
}

void wxExFindReplaceData::SetFindStrings(
  const std::list < wxString > & value)
{
  m_FindStrings = value;

  wxFindReplaceData::SetFindString(m_FindStrings.front());

  SetFindRegularExpression();
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
  wxFindReplaceData::SetReplaceString(value);

  m_ReplaceStrings.remove(value);
  m_ReplaceStrings.push_front(value);
}

void wxExFindReplaceData::SetReplaceStrings(
  const std::list < wxString > & value)
{
  m_ReplaceStrings = value;

  wxFindReplaceData::SetReplaceString(m_ReplaceStrings.front());
}
