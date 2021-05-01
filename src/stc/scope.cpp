////////////////////////////////////////////////////////////////////////////////
// Name:      scope.cpp
// Purpose:   Implementation of class wex::scope
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include <wex/log.h>
#include <wex/stc.h>

#include "scope.h"

wex::scope::scope(stc* s)
  : m_stc(s)
{
  check_levels(check_t().set(LEVEL_UP));

  m_it = m_filters[m_level].end();
}

bool wex::scope::check_levels(check_t type)
{
  const auto size(m_filters.size());
  bool       changed = false;

  if (type[LEVEL_DOWN] && m_level < size - 1)
  {
    m_filters.erase(m_filters.begin() + m_level + 1, m_filters.end());
    changed = true;

    log::trace("scope::erased")
      << size - m_filters.size() << "current:" << m_level
      << "size:" << m_filters.size();
  }

  if (!changed && type[LEVEL_UP] && m_level >= size)
  {
    map_t m;
    m[std::string()] = ctags_entry();

    m_filters.insert(m_filters.end(), m_level - size + 1, m);
    changed = true;

    log::trace("scope::inserted")
      << m_filters.size() - size << "current:" << m_level
      << "size:" << m_filters.size();
  }

  return changed;
}

const std::string wex::scope::class_name(const std::string& name) const
{
  if (const auto& it = m_filters[get_current_level()].find(name);
      it != m_filters[get_current_level()].end())
  {
    return it->second.class_name();
  }

  return std::string();
}

bool wex::scope::end() const
{
  return m_level >= m_filters.size() || m_it == m_filters[m_level].end();
}

bool wex::scope::find(const std::string& text)
{
  if (text.empty())
  {
    return false;
  }

  for (int i = std::min(m_level, m_filters.size() - 1); i >= 0; i--)
  {
    if (const auto& it = m_filters[i].find(text); it != m_filters[i].end())
    {
      m_it = it;
      return true;
    }
  }

  return false;
}

wex::ctags_entry& wex::scope::get(const std::string& text)
{
  return m_filters[m_level][text];
}

size_t wex::scope::get_current_level() const
{
  return std::max(0, m_stc->get_fold_level());
}

void wex::scope::insert(const std::string& text, const ctags_entry& ce)
{
  assert(!text.empty());

  m_filters[m_level].insert({text, ce});

  find(text);
}

void wex::scope::sync()
{
  m_level = get_current_level();

  if (check_levels())
  {
    m_it = m_filters[m_level].end();
  }
}
