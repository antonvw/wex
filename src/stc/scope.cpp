////////////////////////////////////////////////////////////////////////////////
// Name:      scope.cpp
// Purpose:   Implementation of class wex::scope
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/stc/stc.h>

#include "scope.h"

#include <algorithm>

wex::scope::scope(stc* s)
  : m_stc(s)
{
}

bool wex::scope::check_levels()
{
  bool       changed = false;
  const auto level(m_stc->get_fold_level());
  const auto size(m_filters.size());

  if (size >= 1 && level < size - 1)
  {
    m_filters.erase(m_filters.begin() + level + 1, m_filters.end());
    changed = true;

    log::debug("scope::check_levels erased level")
      << size - m_filters.size() << "current:" << level
      << "size:" << m_filters.size();
  }

  if (!changed && level >= size)
  {
    map_t m;
    m[std::string()] = ctags_entry();

    m_filters.insert(m_filters.end(), level - size + 1, m);
    changed = true;

    log::debug("scope::check_levels inserted level")
      << m_filters.size() - size << "current:" << level
      << "size:" << m_filters.size();
  }

  if (changed)
  {
    m_it = m_filters[level].end();
  }

  return changed;
}

const std::string wex::scope::class_name(const std::string& name) const
{
  const auto level(m_stc->get_fold_level());

  if (m_filters.empty() || level >= m_filters.size())
  {
    return std::string();
  }

  log::debug("scope::class_name") << name << level << m_filters[level].size();

  if (const auto& it = iterator(name); !end())
  {
    return it->second.class_name();
  }
  else
  {
    return std::string();
  }
}

bool wex::scope::end() const
{
  const auto level(m_stc->get_fold_level());
  return level >= m_filters.size() || m_it == m_filters[level].end();
}

bool wex::scope::find(const std::string& text)
{
  m_it = iterator(text);
  return !end();
}

wex::ctags_entry& wex::scope::get(const std::string& text)
{
  if (m_filters.empty())
  {
    sync();
  }

  const auto level(m_stc->get_fold_level());
  return m_filters[level][text];
}

void wex::scope::insert(const std::string& text, const ctags_entry& ce)
{
  assert(!text.empty());

  const auto level(m_stc->get_fold_level());

  m_filters[level].insert({text, ce});

  find(text);

  log::debug("scope::insert") << text << level << ce;
}

wex::scope::map_t::const_iterator
wex::scope::iterator(const std::string& text) const
{
  const auto level(m_stc->get_fold_level());

  if (text.empty())
  {
    return m_filters[level].end();
  }

  for (int i = std::min(level, m_filters.size() - 1); i >= 0; i--)
  {
    if (const auto& it = m_filters[i].find(text); it != m_filters[i].end())
    {
      return it;
    }
  }

  return m_filters[level].end();
}

bool wex::scope::sync()
{
  return check_levels();
}
