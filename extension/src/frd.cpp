////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h> 
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

#define SET_STRING( ACTION, STRINGS, VALUE, TEXT)             \
  wxFindReplaceData::Set##ACTION##String(VALUE);              \
  STRINGS.remove(VALUE);                                      \
  STRINGS.push_front(VALUE);                                  \
  wxExListToConfig(STRINGS, TEXT);                            

#define SET_STRINGS( ACTION, STRINGS, VALUE, TEXT)            \
  STRINGS = VALUE;                                            \
                                                              \
  if (!STRINGS.empty())                                       \
  {                                                           \
    wxFindReplaceData::Set##ACTION##String(STRINGS.front());  \
  }                                                           \
  else                                                        \
  {                                                           \
    wxFindReplaceData::Set##ACTION##String(wxEmptyString);    \
  }                                                           \
                                                              \
  wxExListToConfig(STRINGS, TEXT);                            


wxExFindReplaceData* wxExFindReplaceData::m_Self = NULL;

wxExFindReplaceData::wxExFindReplaceData()
  : wxFindReplaceData()
  , m_UseRegEx(false)
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
  SetUseRegEx(wxConfigBase::Get()->ReadBool(m_TextRegEx, m_UseRegEx));
  SetFindStrings(wxExListFromConfig(m_TextFindWhat));
  SetReplaceStrings(wxExListFromConfig(m_TextReplaceWith));
}

wxExFindReplaceData::~wxExFindReplaceData()
{
  wxExListToConfig(m_FindStrings, m_TextFindWhat);
  wxExListToConfig(m_ReplaceStrings, m_TextReplaceWith);

  wxConfigBase::Get()->Write(m_TextMatchCase, MatchCase());
  wxConfigBase::Get()->Write(m_TextMatchWholeWord, MatchWord());
  wxConfigBase::Get()->Write(m_TextRegEx, m_UseRegEx);
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
  
bool wxExFindReplaceData::RegExMatches(const std::string& text) const
{
  return std::regex_search(text, m_FindRegEx, std::regex_constants::format_default);
}
  
int wxExFindReplaceData::RegExReplaceAll(std::string& text) const
{
  auto words_begin = std::sregex_iterator(text.begin(), text.end(), m_FindRegEx);
  auto words_end = std::sregex_iterator();  

  const int result = std::distance(words_begin, words_end);
  
  text = std::regex_replace(text, 
    m_FindRegEx, 
    GetReplaceString().ToStdString(),
    std::regex_constants::format_default);

  return result;
}
  
wxExFindReplaceData* wxExFindReplaceData::Set(wxExFindReplaceData* frd)
{
  wxExFindReplaceData* old = m_Self;
  m_Self = frd;
  return old;
}

void wxExFindReplaceData::SetFindString(const wxString& value)
{
  if (value.empty())
  {
    return;
  }
  
  SET_STRING( Find, m_FindStrings, value, m_TextFindWhat);
  
  SetUseRegEx(m_UseRegEx);
  
  m_FindsIterator = GetFindStrings().begin();
}

void wxExFindReplaceData::SetFindStrings(
  const std::list < wxString > & value)
{
  SET_STRINGS( Find, m_FindStrings, value, m_TextFindWhat);
  
  m_FindsIterator = m_FindStrings.begin();

  SetUseRegEx(m_UseRegEx);
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
    SetUseRegEx(false);
  }
}

void wxExFindReplaceData::SetReplaceString(const wxString& value)
{
  SET_STRING( Replace, m_ReplaceStrings, value, m_TextReplaceWith);
}

void wxExFindReplaceData::SetReplaceStrings(
  const std::list < wxString > & value)
{
  SET_STRINGS( Replace, m_ReplaceStrings, value, m_TextReplaceWith);
}

void wxExFindReplaceData::SetUseRegEx(bool value) 
{
  if (!value)
  {
    m_UseRegEx = false;
    return;
  }
  
  try 
  {
    std::regex::flag_type flags = std::regex::basic;
    if (!MatchCase()) flags |= std::regex::icase;
  
    m_FindRegEx = std::regex(GetFindString().ToStdString(), flags);
    m_UseRegEx = true;
  }
  catch (std::regex_error& e) 
  {
    m_UseRegEx = false;
    wxLogStatus("regex error: %s %s", e.what(), GetFindString().ToStdString());
  }
}

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
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (!wxExFindReplaceData::Get()->Iterate(this, event.GetKeyCode()))
    {
      event.Skip();
    }});
  
  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetFindString(GetValue());});
}
