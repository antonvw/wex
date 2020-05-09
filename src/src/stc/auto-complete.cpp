////////////////////////////////////////////////////////////////////////////////
// Name:      auto-complete.cpp
// Purpose:   Implementation of class wex::auto_complete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "scope.h"
#include <wex/auto-complete.h>
#include <wex/config.h>
#include <wex/ctags.h>
#include <wex/log.h>
#include <wex/stc.h>

wex::auto_complete::auto_complete(wex::stc* stc)
  : m_stc(stc)
  , m_min_size(3)
  , m_scope(new scope(stc))
{
}

wex::auto_complete::~auto_complete()
{
  delete m_scope;
}

bool wex::auto_complete::activate(const std::string& text)
{
  if (text.empty() || !use())
  {
    return false;
  }

  // Find suitable entry.
  ctags_entry& ce = m_scope->get(text);

  // Fills the entry.
  m_stc->get_vi().ctags()->find(text, ce);

  if (ce.is_active())
  {
    log::verbose("activated filter", 2) << text << ce;
    m_active = text;
  }

  if (m_stc->get_vi().is_active())
  {
    m_stc->get_vi().append_insert_text(text.substr(m_text.size()));
  }

  m_text.clear();

  return true;
}

void wex::auto_complete::clear()
{
  m_text.clear();

  m_stc->AutoCompCancel();
}

bool wex::auto_complete::on_char(char c)
{
  if (!use() || m_stc->SelectionIsRectangle())
  {
    return false;
  }

  bool shw_inserts  = true;
  bool shw_keywords = true;

  if (
    c == '.' ||
    (c == '>' && m_stc->GetCharAt(m_stc->GetCurrentPos() - 1) == '-'))
  {
    m_text.clear();
    m_get_members = true;

    shw_inserts  = false;
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
  else if (c == WXK_RETURN)
  {
    m_scope->sync();
    return false;
  }
  else if (isspace(c))
  {
    if (m_text.size() > m_min_size)
    {
      m_inserts.emplace(m_text);
    }

    clear();
    return false;
  }
  else if (iscntrl(c) || c == '+')
  {
    return false;
  }
  // scope end
  else if (c == ';')
  {
    clear();
    m_active.clear();
    return false;
  }
  else
  {
    m_get_members = false;

    if (is_codeword_separator(m_stc->GetCharAt(m_stc->GetCurrentPos() - 1)))
    {
      m_text = c;
    }
    else
    {
      m_text += c;
    }
  }

  if (const auto wsp = m_stc->WordStartPosition(m_stc->GetCurrentPos(), true);
      (m_stc->GetCharAt(wsp - 1) == '.') ||
      (m_stc->GetCharAt(wsp - 1) == '>' && m_stc->GetCharAt(wsp - 2) == '-'))
  {
    shw_inserts  = false;
    shw_keywords = false;
  }

  if (
    !show_ctags(true) && !show_inserts(shw_inserts) &&
    !show_keywords(shw_keywords))
  {
    m_stc->AutoCompCancel();
  }

  return true;
}

bool wex::auto_complete::show_ctags(bool show)
{
  if (!show)
  {
    return false;
  }

  const std::string use_filter = (!m_text.empty() ? m_text : m_active);

  bool as_variable = false;

  // If members are requested, and class is active, save it in the filters.
  if (m_get_members && m_scope->find(use_filter))
  {
    const auto& ce = m_scope->iter()->second; // backup
    const auto  wsp(m_stc->WordStartPosition(m_stc->GetCurrentPos() - 1, true));
    const std::string word(m_stc->GetTextRange(wsp, m_stc->GetCurrentPos()));

    if (!m_active.empty() && !m_scope->find(word))
    {
      m_scope->insert(word, ce);
    }

    if (!m_scope->end())
    {
      as_variable = true;
      log::verbose("variable", 2) << word << m_scope->iter()->second;
    }
  }

  if (!m_scope->end() && !as_variable)
  {
    log::verbose("use filter", 2)
      << m_scope->iter()->first << m_scope->iter()->second
      << "from:" << use_filter;
  }

  // Use iterator as filter (const).
  if (const auto comp(m_stc->get_vi().ctags()->auto_complete(
        m_text,
        !m_scope->end() ? m_scope->iter()->second : ctags_entry()));
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

bool wex::auto_complete::show_inserts(bool show) const
{
  if (show && !m_text.empty() && !m_inserts.empty())
  {
    if (const auto comp(get_string_set(m_inserts, m_min_size, m_text));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_text.length() - 1, comp);
      log::verbose("auto_complete::show_inserts chars") << comp.size();
      return true;
    }
  }

  return false;
}

bool wex::auto_complete::show_keywords(bool show) const
{
  if (show && !m_text.empty() && m_stc->get_lexer().keyword_starts_with(m_text))
  {
    if (const auto comp(
          m_stc->get_lexer().keywords_string(-1, m_min_size, m_text));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_text.length() - 1, comp);
      log::verbose("auto_complete::show_keywords chars") << comp.size();
      return true;
    }
  }

  return false;
}

void wex::auto_complete::sync() const
{
  m_scope->sync();
}

bool wex::auto_complete::use() const
{
  return m_use && config(_("stc.Auto complete")).get(false);
}
