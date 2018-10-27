////////////////////////////////////////////////////////////////////////////////
// Name:      autocomplete.cpp
// Purpose:   Implementation of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/autocomplete.h>
#include <wex/config.h>
#include <wex/ctags.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <easylogging++.h>

wex::autocomplete::autocomplete(wex::stc* stc)
  : m_STC(stc)
  , m_MinSize(3)
{
}

bool wex::autocomplete::Activate(const std::string& text)
{
  if (text.empty() || !Use())
  {
    return false;
  }

  wex::ctags_entry current;

  m_STC->GetVi().GetCTags()->Find(text, current, m_Filter);

  VLOG(9) << "autocomplete: " << text;
  
  if (current.Active())
  {
    VLOG(9) << "autocomplete current: " << current.Get();
  }

  if (m_Filter.Active())
  {
    VLOG(9) << "autocomplete filter: " << m_Filter.Get();
  }

  if (m_STC->GetVi().GetIsActive())
  {
    m_STC->GetVi().AppendInsertText(text.substr(m_Text.size()));
  }

  m_Text.clear();

  return true;
}

bool wex::autocomplete::Apply(char c)
{
  if (!Use() || m_STC->SelectionIsRectangle())
  {
    return false;
  }

  bool show_ctags = true;
  bool show_inserts = true;
  bool show_keywords = true;
  
  if (c == '.' || 
     (c == '>' && m_STC->GetCharAt(m_STC->GetCurrentPos() - 1) == '-'))
  {
    Clear();
    show_inserts = false;
    show_keywords = false;
  }

  else if (c == WXK_BACK)
  {
    if (m_Text.empty())
    {
      return false;
    }

    m_Text.pop_back();
  }
  else if (isspace(c))
  {
    if (m_Text.size() > m_MinSize)
    {
      m_Inserts.emplace(m_Text);
    }

    Clear();
    return true;
  }
  else if (iscntrl(c) || c == '+')
  {
    return false;
  }
  else
  {
    if (is_codeword_separator(
      m_STC->GetCharAt(m_STC->GetCurrentPos() - 1)))
    {
      m_Text = c;
    }
    else
    {
      m_Text += c;
    }
  }

  if (
    !ShowCTags(show_ctags) &&
    !ShowInserts(show_inserts) &&
    !ShowKeywords(show_keywords))
  {
    m_STC->AutoCompCancel();
  }

  return true;
}

void wex::autocomplete::Clear()
{
  m_Text.clear();
  m_STC->AutoCompCancel();
}

void wex::autocomplete::Reset()
{
  m_Filter.Clear();
}

bool wex::autocomplete::ShowCTags(bool show) const
{
  if (!show) 
  {
    return false;
  }

  if (const auto comp(m_STC->GetVi().GetCTags()->AutoComplete(
    m_Text, m_Filter));
    comp.empty())
  {
    return false;
  }
  else 
  {
    m_STC->AutoCompSetSeparator(m_STC->GetVi().GetCTags()->Separator());
    m_STC->AutoCompShow(m_Text.length() - 1, comp);
    m_STC->AutoCompSetSeparator(' ');
    return true;
  }
}

bool wex::autocomplete::ShowInserts(bool show) const
{
  if (show && !m_Text.empty() && !m_Inserts.empty())
  {
    if (const auto comp(get_string_set(
      m_Inserts, m_MinSize, m_Text));
      !comp.empty())
    {
      m_STC->AutoCompShow(m_Text.length() - 1, comp);
      return true;
    }
  }

  return false;
}

bool wex::autocomplete::ShowKeywords(bool show) const
{
  if (show && !m_Text.empty() && m_STC->GetLexer().KeywordStartsWith(m_Text))
  {
    if (const auto comp(
      m_STC->GetLexer().GetKeywordsString(-1, m_MinSize, m_Text));
      !comp.empty())
    {
      m_STC->AutoCompShow(m_Text.length() - 1, comp);
      return true;
    }
  }

  return false;
}

bool wex::autocomplete::Use() const
{
  return m_Use || config(_("Auto complete")).get(false);
}
