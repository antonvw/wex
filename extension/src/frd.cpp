////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h> 
#include <wx/extension/frd.h>
#include <wx/extension/ex-command.h>
#include <wx/extension/util.h>

wxExFindReplaceData* wxExFindReplaceData::m_Self = nullptr;
std::string wxExFindReplaceData::m_TextFindWhat = _("Find what").ToStdString();
std::string wxExFindReplaceData::m_TextMatchCase = _("Match case").ToStdString();
std::string wxExFindReplaceData::m_TextMatchWholeWord = _("Match whole word").ToStdString();
std::string wxExFindReplaceData::m_TextRegEx = _("Regular expression").ToStdString();
std::string wxExFindReplaceData::m_TextReplaceWith = _("Replace with").ToStdString();
std::string wxExFindReplaceData::m_TextSearchDown = _("Search down").ToStdString();

wxExFindReplaceData::wxExFindReplaceData()
  : m_FindStrings(wxExExCommandType::FIND)
  , m_ReplaceStrings(wxExExCommandType::REPLACE)
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
  wxConfigBase::Get()->Write(m_TextMatchCase, MatchCase());
  wxConfigBase::Get()->Write(m_TextMatchWholeWord, MatchWord());
  wxConfigBase::Get()->Write(m_TextRegEx, m_UseRegEx);
  wxConfigBase::Get()->Write(m_TextSearchDown, SearchDown());
}

wxExFindReplaceData* wxExFindReplaceData::Get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new wxExFindReplaceData();
  }

  return m_Self;
}

int wxExFindReplaceData::RegExMatches(const std::string& text) const
{
  std::smatch m;

  if (!std::regex_search(
    text, 
    m,
    m_FindRegEx, std::regex_constants::format_default)) return -1;

  return m.position();
}
  
int wxExFindReplaceData::RegExReplaceAll(std::string& text) const
{
  const auto words_begin = std::sregex_iterator(text.begin(), text.end(), m_FindRegEx);
  const auto words_end = std::sregex_iterator();  
  const int result = std::distance(words_begin, words_end);
  
  text = std::regex_replace(text, 
    m_FindRegEx, 
    GetReplaceString(),
    std::regex_constants::format_default);

  return result;
}
  
wxExFindReplaceData* wxExFindReplaceData::Set(wxExFindReplaceData* frd)
{
  wxExFindReplaceData* old = m_Self;
  m_Self = frd;
  return old;
}

void wxExFindReplaceData::SetFindString(const std::string& value)
{
  if (!m_FindStrings.Set(value)) return;
  m_FRD.SetFindString(value);
  SetUseRegEx(m_UseRegEx);
}

void wxExFindReplaceData::SetFindStrings(
  const std::list < std::string > & values)
{
  m_FindStrings.Set(values);
  m_FRD.SetFindString(m_FindStrings.Get());
  SetUseRegEx(m_UseRegEx);
}

void wxExFindReplaceData::SetMatchCase(bool value)
{
  auto flags = GetFlags();
  if (value) flags |= wxFR_MATCHCASE;
  else       flags &= ~wxFR_MATCHCASE;
  SetFlags(flags);
}

void wxExFindReplaceData::SetMatchWord(bool value)
{
  auto flags = GetFlags();
  
  if (value) flags |= wxFR_WHOLEWORD;
  else       flags &= ~wxFR_WHOLEWORD;
  
  SetFlags(flags);
  
  // Match word and regular expression do not work together.
  if (value)
  {
    SetUseRegEx(false);
  }
}

void wxExFindReplaceData::SetReplaceString(const std::string& value)
{
  m_ReplaceStrings.Set(value);
  m_FRD.SetReplaceString(value);
}

void wxExFindReplaceData::SetReplaceStrings(
  const std::list < std::string > & value)
{
  m_ReplaceStrings.Set(value);
  m_FRD.SetReplaceString(m_ReplaceStrings.Get());
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
    std::regex::flag_type flags = std::regex::ECMAScript;
    if (!MatchCase()) flags |= std::regex::icase;
  
    m_FindRegEx = std::regex(GetFindString(), flags);
    m_UseRegEx = true;
  }
  catch (std::regex_error& e) 
  {
    m_UseRegEx = false;
    wxLogStatus("regex error: %s %s", e.what(), GetFindString());
  }
}
