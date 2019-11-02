////////////////////////////////////////////////////////////////////////////////
// Name:      autocomplete.cpp
// Purpose:   Implementation of class wex::autocomplete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/autocomplete.h>
#include <wex/config.h>
#include <wex/ctags.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/util.h>

wex::autocomplete::autocomplete(wex::stc* stc)
  : m_stc(stc)
  , m_min_size(3)
{
}

bool wex::autocomplete::activate(const std::string& text)
{
  if (text.empty() || !use())
  {
    return false;
  }

  wex::ctags_entry current;

  m_stc->get_vi().ctags()->find(text, current, m_filter);

  log::verbose("autocomplete") << text;
  
  if (current.is_active())
  {
    log::verbose("autocomplete current") << current;
  }

  if (m_filter.is_active())
  {
    log::verbose("autocomplete filter") << m_filter;
  }

  if (m_stc->get_vi().is_active())
  {
    m_stc->get_vi().append_insert_text(text.substr(m_text.size()));
  }

  m_text.clear();

  return true;
}

bool wex::autocomplete::apply(char c)
{
  if (!use() || m_stc->SelectionIsRectangle())
  {
    return false;
  }

  bool shw_inserts = true;
  bool shw_keywords = true;
  
  if (c == '.' || 
     (c == '>' && m_stc->GetCharAt(m_stc->GetCurrentPos() - 1) == '-'))
  {
    clear();
    shw_inserts = false;
    shw_keywords = false;
  }
  else if (c == WXK_BACK)
  {
    if (m_text.empty())
    {
      return false;
    }

    m_text.pop_back();
  }
  else if (isspace(c))
  {
    if (m_text.size() > m_min_size)
    {
      m_inserts.emplace(m_text);
    }

    clear();
    return true;
  }
  else if (iscntrl(c) || c == '+')
  {
    return false;
  }
  else
  {
    if (is_codeword_separator(
      m_stc->GetCharAt(m_stc->GetCurrentPos() - 1)))
    {
      m_text = c;
    }
    else
    {
      m_text += c;
    }
  }
    
  if (const auto wsp = m_stc->WordStartPosition(m_stc->GetCurrentPos(), true);
      (m_stc->GetCharAt(wsp -1) == '.') ||
      (m_stc->GetCharAt(wsp -1) == '>' && m_stc->GetCharAt(wsp - 2) == '-'))
  {
    shw_inserts = false;
    shw_keywords = false;
  }

  if (
    !show_ctags(true) &&
    !show_inserts(shw_inserts) &&
    !show_keywords(shw_keywords))
  {
    m_stc->AutoCompCancel();
  }

  return true;
}

void wex::autocomplete::clear()
{
  m_text.clear();
  m_stc->AutoCompCancel();
}

void wex::autocomplete::reset()
{
  m_filter.clear();
}

bool wex::autocomplete::show_ctags(bool show) const
{
  if (!show) 
  {
    return false;
  }

  if (const auto comp(m_stc->get_vi().ctags()->autocomplete(
    m_text, m_filter));
    comp.empty())
  {
    return false;
  }
  else 
  {
    m_stc->AutoCompSetSeparator(m_stc->get_vi().ctags()->separator());
    m_stc->AutoCompShow(m_text.length() - 1, comp);
    m_stc->AutoCompSetSeparator(' ');
    return true;
  }
}

bool wex::autocomplete::show_inserts(bool show) const
{
  if (show && !m_text.empty() && !m_inserts.empty())
  {
    if (const auto comp(get_string_set(
      m_inserts, m_min_size, m_text));
      !comp.empty())
    {
      m_stc->AutoCompShow(m_text.length() - 1, comp);
      log::verbose("autocomplete::show_insert chars") << comp.size();
      return true;
    }
  }

  return false;
}

bool wex::autocomplete::show_keywords(bool show) const
{
  if (show && !m_text.empty() && m_stc->get_lexer().keyword_starts_with(m_text))
  {
    if (const auto comp(
      m_stc->get_lexer().keywords_string(-1, m_min_size, m_text));
      !comp.empty())
    {
      m_stc->AutoCompShow(m_text.length() - 1, comp);
      log::verbose("autocomplete::show_keywords chars") << comp.size();
      return true;
    }
  }

  return false;
}

bool wex::autocomplete::use() const
{
  return m_use && config(_("Auto complete")).get(false);
}
