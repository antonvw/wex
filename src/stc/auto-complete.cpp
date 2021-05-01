////////////////////////////////////////////////////////////////////////////////
// Name:      auto-complete.cpp
// Purpose:   Implementation of class wex::auto_complete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/auto-complete.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/ctags.h>
#include <wex/log.h>
#include <wex/stc.h>

#include "scope.h"

wex::auto_complete::auto_complete(wex::stc* stc, size_t min_size)
  : m_stc(stc)
  , m_min_size(min_size)
  , m_scope(new scope(stc))
{
}

wex::auto_complete::~auto_complete()
{
  delete m_scope;
}

void wex::auto_complete::clear()
{
  if (m_insert.size() >= m_min_size)
  {
    m_inserts.emplace(m_insert);
  }

  m_insert.clear();

  m_stc->AutoCompCancel();
}

bool wex::auto_complete::complete(const std::string& text)
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
    log::trace("activated filter") << text << ce;
    m_active = text;
  }

  if (m_stc->get_vi().is_active())
  {
    m_stc->get_vi().append_insert_text(text.substr(m_insert.size()));
  }

  m_insert.clear();

  return true;
}

size_t wex::auto_complete::current_level()
{
  return m_scope->get_current_level();
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
    m_insert.clear();
    m_request_members = (c == '>' ? "->" : ".");

    shw_inserts  = false;
    shw_keywords = false;
  }
  else if (c == WXK_BACK)
  {
    if (m_insert.empty())
    {
      return false;
    }

    m_insert.pop_back();
  }
  else if (c == WXK_RETURN)
  {
    m_scope->sync();
    return false;
  }
  else if (isspace(c))
  {
    if (m_insert.size() > m_min_size)
    {
      m_inserts.emplace(m_insert);
    }

    clear();
    return false;
  }
  else if (iscntrl(c) || c == '+')
  {
    return false;
  }
  else if (c == ';')
  {
    // active end
    clear();
    m_active.clear();
    return false;
  }
  else if (c == '}')
  {
    // scope end
    clear();
    m_scope->sync();
    m_active.clear();
    m_request_members.clear();
    return false;
  }
  else
  {
    m_request_members.clear();

    if (is_codeword_separator(m_stc->GetCharAt(m_stc->GetCurrentPos() - 1)))
    {
      m_insert = c;
    }
    else
    {
      m_insert += c;
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
    !show_ctags() && !show_inserts(shw_inserts) && !show_keywords(shw_keywords))
  {
    m_stc->AutoCompCancel();
  }

  return true;
}

bool wex::auto_complete::show_ctags()
{
  // If members are requested, and class is active, save it in the filters.
  if (const auto& use_filter = (!m_insert.empty() ? m_insert : m_active);
      !m_request_members.empty() && m_scope->find(use_filter))
  {
    const auto&       ce = m_scope->iter()->second; // backup
    const auto        wsp(m_stc->WordStartPosition(
      m_stc->GetCurrentPos() - m_request_members.size(),
      true));
    const std::string word(m_stc->GetTextRange(wsp, m_stc->GetCurrentPos()));

    if (!m_active.empty() && !m_scope->find(word))
    {
      m_scope->insert(word, ce);
    }
  }

  // Use iterator as filter (const).
  if (const auto& comp(m_stc->get_vi().ctags()->auto_complete(
        m_insert,
        !m_scope->end() && !m_active.empty() ? m_scope->iter()->second :
                                               ctags_entry()));
      !comp.empty())
  {
    m_stc->AutoCompSetSeparator(m_stc->get_vi().ctags()->separator());
    m_stc->AutoCompShow(m_insert.length() - 1, comp);
    m_stc->AutoCompSetSeparator(' ');
    return true;
  }
  else
  {
    return false;
  }
}

bool wex::auto_complete::show_inserts(bool show) const
{
  if (show && !m_insert.empty() && !m_inserts.empty())
  {
    if (const auto comp(get_string_set(m_inserts, m_min_size, m_insert));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_insert.length() - 1, comp);
      log::trace("auto_complete::show_inserts chars") << comp.size();
      return true;
    }
  }

  return false;
}

bool wex::auto_complete::show_keywords(bool show) const
{
  if (
    show && !m_insert.empty() &&
    m_stc->get_lexer().keyword_starts_with(m_insert))
  {
    if (const auto comp(
          m_stc->get_lexer().keywords_string(-1, m_min_size, m_insert));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_insert.length() - 1, comp);
      log::trace("auto_complete::show_keywords chars") << comp.size();
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

const std::string wex::auto_complete::variable(const std::string& name) const
{
  return m_scope->class_name(name);
}
