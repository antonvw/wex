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
  clear_insert();

  m_request_members.clear();
  m_stc->AutoCompCancel();
}

void wex::auto_complete::clear_insert()
{
  if (m_insert.size() >= m_min_size)
  {
    m_inserts.emplace(m_insert);
    log::debug("auto_complete::clear inserts") << m_insert;
  }

  m_insert.clear();
}

bool wex::auto_complete::complete(const std::string& text)
{
  if (text.empty() || !use())
  {
    return false;
  }

  // Find suitable entry.
  auto& ce = m_scope->get(text);

  // Fills the entry.
  bool result = ctags::find(text, ce);

  if (ce.is_active())
  {
    m_active = text;
    log::debug("auto_complete::complete active") << m_active << ce;
  }
  else
  {
    log::debug("auto_complete::complete") << text << result;
  }

  if (m_stc->get_vi().is_active())
  {
    m_stc->get_vi().append_insert_text(text.substr(m_insert.size()));
  }

  // do not add current insert
  m_insert.clear();

  return true;
}

bool wex::auto_complete::on_char(char c)
{
  if (!use() || m_stc->SelectionIsRectangle())
  {
    return false;
  }

  bool shw_inserts  = true;
  bool shw_keywords = true;

  m_scope->sync();

  switch (c)
  {
    case '.':
    case '>':
      if (
        c == '.' ||
        (c == '>' && m_stc->GetCharAt(m_stc->GetCurrentPos() - 1) == '-'))
      {
        if (!m_insert.empty())
        {
          m_request_members = (c == '>' ? "->" : ".");

          if (!m_active.empty())
          {
            clear_insert();
          }
        }

        shw_inserts  = false;
        shw_keywords = false;
      }
      break;

    case WXK_BACK:
      switch (m_insert.size())
      {
        case 0:
          return false;

        case 1:
          m_insert.pop_back();
          return false;

        default:
          m_insert.pop_back();
      }
      break;

    case WXK_RETURN:
      return false;

    case ',':
      store_variable();
      return false;

    case ';':
      store_variable();

      // active end
      clear();
      m_active.clear();
      return false;

    case '{':
      // scope start
      clear();
      m_request_members.clear();
      return false;

    default:
      if (is_codeword_separator(c) || iscntrl(c))
      {
        clear_insert();
        clear();
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
    return false;
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

    clear_insert();

    if (!m_active.empty() && !m_scope->find(word))
    {
      m_scope->insert(word, ce);
    }
  }

  // Use iterator as filter (const).
  if (const auto& comp(m_stc->get_vi().ctags()->auto_complete(
        m_insert,
        !m_scope->end() && !m_request_members.empty() ?
          m_scope->iter()->second :
          ctags_entry()));
      !comp.empty())
  {
    m_stc->AutoCompSetSeparator(m_stc->get_vi().ctags()->separator());
    m_stc->AutoCompShow(m_insert.length() - 1, comp);
    m_stc->AutoCompSetSeparator(' ');
    log::debug("auto_complete::show_ctags chars") << m_insert << comp.size();
    return true;
  }

  return false;
}

bool wex::auto_complete::show_inserts(bool show) const
{
  if (show && !m_insert.empty() && !m_inserts.empty())
  {
    if (const auto& comp(get_string_set(m_inserts, m_min_size, m_insert));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_insert.length() - 1, comp);
      log::debug("auto_complete::show_inserts chars") << comp.size();
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
    if (const auto& comp(
          m_stc->get_lexer().keywords_string(-1, m_min_size, m_insert));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_insert.length() - 1, comp);
      log::debug("auto_complete::show_keywords chars") << comp.size();
      return true;
    }
  }

  return false;
}

void wex::auto_complete::store_variable()
{
  if (!m_active.empty() && !m_insert.empty())
  {
    const ctags_entry& ce = m_scope->get(m_active);
    m_scope->insert(m_insert, ce);
  }
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
