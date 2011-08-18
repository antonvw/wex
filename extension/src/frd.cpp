////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h> 
#include <wx/checklst.h> 
#include <wx/stc/stc.h> 
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
  SetUseRegularExpression(wxConfigBase::Get()->ReadBool(m_TextRegEx, false));

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
  wxConfigBase::Get()->Write(m_TextRegEx, m_UseRegularExpression);
  wxConfigBase::Get()->Write(m_TextSearchDown, SearchDown());
}

int wxExFindReplaceData::STCFlags() const
{
  int flags = 0;

  if (UseRegularExpression())  flags |= wxSTC_FIND_REGEXP;
  if (MatchWord()) flags |= wxSTC_FIND_WHOLEWORD;
  if (MatchCase()) flags |= wxSTC_FIND_MATCHCASE;

  return flags;
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
    clb->Check(item, UseRegularExpression());
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
    log += " " + _("replacing with") + ": " + 
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
    SetUseRegularExpression(value);
  }
  else
  {
    // No wxFAIL, see configitem.
    return false;
  }

  return true;
}

void wxExFindReplaceData::SetFindRegularExpression()
{
  if (UseRegularExpression())
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

  wxExListToConfig(m_FindStrings, m_TextFindWhat);

  SetFindRegularExpression();
}

void wxExFindReplaceData::SetFindStrings(
  const std::list < wxString > & value)
{
  m_FindStrings = value;

  wxFindReplaceData::SetFindString(m_FindStrings.front());

  wxExListToConfig(m_FindStrings, m_TextFindWhat);

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
  
  // Match word and regular expression do not work together.
  if (value)
  {
    SetUseRegularExpression(false);
  }
}

void wxExFindReplaceData::SetReplaceString(const wxString& value)
{
  wxFindReplaceData::SetReplaceString(value);

  m_ReplaceStrings.remove(value);
  m_ReplaceStrings.push_front(value);

  wxExListToConfig(m_ReplaceStrings, m_TextReplaceWith);
}

void wxExFindReplaceData::SetReplaceStrings(
  const std::list < wxString > & value)
{
  m_ReplaceStrings = value;

  wxFindReplaceData::SetReplaceString(m_ReplaceStrings.front());

  wxExListToConfig(m_ReplaceStrings, m_TextReplaceWith);
}
