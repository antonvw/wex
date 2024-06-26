////////////////////////////////////////////////////////////////////////////////
// Name:      auto-complete.cpp
// Purpose:   Implementation of class wex::auto_complete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/ctags/ctags.h>
#include <wex/stc/auto-complete.h>
#include <wex/stc/stc.h>

#include "scope.h"

namespace wex
{
class auto_complete::actions
{
public:
  void reset()
  {
    m_show_inserts  = false;
    m_show_keywords = false;
  };

  bool m_show_inserts{true};
  bool m_show_keywords{true};
};
}; // namespace wex

wex::auto_complete::auto_complete(wex::stc* stc)
  : m_stc(stc)
  , m_scope(new scope(stc))
{
}

wex::auto_complete::~auto_complete()
{
  delete m_scope;
}

bool wex::auto_complete::action_back()
{
  switch (m_insert.size())
  {
    case 0:
      return false;

    case 1:
      m_insert.pop_back();
      return false;

    default:
      m_insert.pop_back();
      return true;
  }
}

bool wex::auto_complete::action_default(char c)
{
  if (is_codeword_separator(c) || iscntrl(c))
  {
    if (!m_stc->AutoCompActive())
    {
      update_inserts();
      clear();
    }

    return false;
  }

  m_request_members.clear();

  if (is_codeword_separator(m_stc->GetCharAt(m_stc->GetCurrentPos() - 1)))
  {
    m_insert = c;
  }
  else
  {
    m_insert += c;
  }

  return true;
}

bool wex::auto_complete::action_request(char c, actions& ac)
{
  if (
    c == '.' ||
    (c == '>' && m_stc->GetCharAt(m_stc->GetCurrentPos() - 1) == '-'))
  {
    if (!m_insert.empty())
    {
      m_request_members = (c == '>' ? "->" : ".");

      if (!m_active.empty())
      {
        update_inserts();
      }
    }

    ac.reset();
  }

  return true;
}

void wex::auto_complete::clear()
{
  update_inserts();

  m_request_members.clear();
  m_stc->AutoCompCancel();
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
  ctags::find(text, ce);

  if (ce.is_active())
  {
    m_active = text;
    log::debug("auto_complete::complete active") << m_active << ce;
  }
  else
  {
    log::debug("auto_complete::complete") << text;
  }

  if (m_stc->get_vi().is_active())
  {
    m_stc->get_vi().append_insert_text(text.substr(m_insert.size()));
  }

  // do not add current insert
  m_insert.clear();

  return true;
}

bool wex::auto_complete::determine_actions(char c, actions& ac)
{
  switch (c)
  {
    case WXK_BACK:
      return action_back();

    case '.':
    case '>':
      return action_request(c, ac);

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
      return action_default(c);
  }

  return true;
}

bool wex::auto_complete::on_char(char c)
{
  if (!use() || m_stc->SelectionIsRectangle())
  {
    return false;
  }

  m_scope->sync();

  actions ac;

  if (!determine_actions(c, ac))
  {
    return false;
  }

  if (const auto wsp = m_stc->WordStartPosition(m_stc->GetCurrentPos(), true);
      (m_stc->GetCharAt(wsp - 1) == '.') ||
      (m_stc->GetCharAt(wsp - 1) == '>' && m_stc->GetCharAt(wsp - 2) == '-'))
  {
    ac.reset();
  }

  if (
    !show_ctags() && !show_inserts(ac.m_show_inserts) &&
    !show_keywords(ac.m_show_keywords))
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

    update_inserts();

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
    log::debug("auto_complete::show_ctags insert")
      << m_insert << "size" << comp.size();
    return true;
  }

  return false;
}

bool wex::auto_complete::show_inserts(bool show) const
{
  if (show && !m_insert.empty() && !m_inserts.empty())
  {
    if (const auto& comp(get_string_set(m_inserts, 0, m_insert)); !comp.empty())
    {
      m_stc->AutoCompShow(m_insert.length() - 1, comp);
      log::debug("auto_complete::show_inserts insert")
        << m_insert << "size" << comp.size();
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
    if (const auto& comp(m_stc->get_lexer().keywords_string(-1, 0, m_insert));
        !comp.empty())
    {
      m_stc->AutoCompShow(m_insert.length() - 1, comp);
      log::debug("auto_complete::show_keywords insert")
        << m_insert << "size" << comp.size();
      return true;
    }
  }

  return false;
}

void wex::auto_complete::store_variable()
{
  if (!m_active.empty() && !m_insert.empty())
  {
    const auto& ce = m_scope->get(m_active);
    m_scope->insert(m_insert, ce);
  }
}

bool wex::auto_complete::sync() const
{
  return m_scope->sync();
}

void wex::auto_complete::update_inserts()
{
  if (m_insert.size() >= config("stc.Autocomplete min size").get(2))
  {
    m_inserts.emplace(m_insert);
    log::debug("auto_complete::update_inserts added") << m_insert;
  }

  m_insert.clear();
}

bool wex::auto_complete::use() const
{
  return m_use && config(_("stc.Auto complete")).get(false);
}

const std::string wex::auto_complete::variable(const std::string& name) const
{
  return m_scope->class_name(name);
}
