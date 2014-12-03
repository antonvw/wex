////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h> 
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
  SetFindStrings(wxExListFromConfig(m_TextFindWhat));
  SetReplaceStrings(wxExListFromConfig(m_TextReplaceWith));
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

wxExFindReplaceData* wxExFindReplaceData::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExFindReplaceData();
  }

  return m_Self;
}

bool wxExFindReplaceData::Iterate(wxTextCtrl* ctrl, int key)
{
  return wxExSetTextCtrlValue(ctrl, key, m_FindStrings, m_FindsIterator);
}
  
wxExFindReplaceData* wxExFindReplaceData::Set(wxExFindReplaceData* frd)
{
  wxExFindReplaceData* old = m_Self;
  m_Self = frd;
  return old;
}

bool wxExFindReplaceData::Set(const wxString& field, bool value)
{
  if (field == m_TextMatchCase)
  {
    SetMatchCase(value);
  }
  else if (field == m_TextMatchWholeWord)
  {
    SetMatchWord(value);
  }
  else if (field == m_TextRegEx)
  {
    SetUseRegularExpression(value);
  }
  else if (field == m_TextSearchDown)
  {
    SetFlags(value ? wxFR_DOWN: ~wxFR_DOWN);
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
  // We always set the regular expression, in the Find In Files
  // dialog, the FindString is invoked before matc regex
  // was set...
  int flags = wxRE_ADVANCED;
  if (!MatchCase()) flags |= wxRE_ICASE;
  m_FindRegularExpression.Compile(GetFindString(), flags);
}

void wxExFindReplaceData::SetFindString(const wxString& value)
{
  if (value.empty())
  {
    return;
  }
  
  wxFindReplaceData::SetFindString(value);

  m_FindStrings.remove(value);
  m_FindStrings.push_front(value);

  wxExListToConfig(m_FindStrings, m_TextFindWhat);

  SetFindRegularExpression();
  
  m_FindsIterator = GetFindStrings().begin();
}

void wxExFindReplaceData::SetFindStrings(
  const std::list < wxString > & value)
{
  m_FindStrings = value;
  
  if (!m_FindStrings.empty())
  {
    wxFindReplaceData::SetFindString(m_FindStrings.front());
  }
  else
  {
    wxFindReplaceData::SetFindString(wxEmptyString);
  }
  
  m_FindsIterator = m_FindStrings.begin();

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

  if (!m_ReplaceStrings.empty())
  {
    wxFindReplaceData::SetReplaceString(m_ReplaceStrings.front());
  }
  else
  {
    wxFindReplaceData::SetReplaceString(wxEmptyString);
  }

  wxExListToConfig(m_ReplaceStrings, m_TextReplaceWith);
}

BEGIN_EVENT_TABLE(wxExFindTextCtrl, wxTextCtrl)
  EVT_CHAR(wxExFindTextCtrl::OnKey)
  EVT_TEXT_ENTER(wxID_ANY, wxExFindTextCtrl::OnEnter)
END_EVENT_TABLE()

wxExFindTextCtrl::wxExFindTextCtrl(
  wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxTextCtrl(parent, 
      id,
      wxExFindReplaceData::Get()->GetFindString(), 
      pos, size, wxTE_PROCESS_ENTER)
{
}

void wxExFindTextCtrl::OnEnter(wxCommandEvent& event)
{
  wxExFindReplaceData::Get()->SetFindString(GetValue());
}

void wxExFindTextCtrl::OnKey(wxKeyEvent& event)
{
  if (!wxExFindReplaceData::Get()->Iterate(this, event.GetKeyCode()))
  {
    event.Skip();
  }
}    
