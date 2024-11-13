////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diffs.cpp
// Purpose:   Implementation of class unified_diffs
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/stc.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/unified-diffs.h>

wex::unified_diffs::unified_diffs(stc* s)
  : m_stc(s)
  , m_lines_it(m_lines.begin())
{
}

void wex::unified_diffs::clear()
{
  m_lines.clear();
  m_lines_it = m_lines.begin();
}

int wex::unified_diffs::distance() const
{
  return std::distance(m_lines.begin(), m_lines_it);
}

bool wex::unified_diffs::first()
{
  if (m_lines.empty())
  {
    return false;
  }

  m_lines_it = m_lines.begin();

  m_stc->goto_line(*m_lines_it);

  return true;
}

void wex::unified_diffs::insert(const unified_diff* diff) 
{ 
  if (diff->range_from_start() != 0)
  {
    m_lines.insert(diff->range_from_start() - 1); 
  }

  if (diff->range_to_start() != 0)
  {
    m_lines.insert(diff->range_to_start() - 1); 
  }
  
  m_lines_it = m_lines.begin();
}

bool wex::unified_diffs::next()
{
  if (m_lines_it == m_lines.end() || 
      m_lines_it == std::prev(m_lines.end()))
  {
    return m_stc->get_vi().command(":n");
  }

  m_lines_it++;

  m_stc->goto_line(*m_lines_it);
  
  return true;
}

bool wex::unified_diffs::prev()
{
  if (m_lines_it == m_lines.begin())
  {
    return m_stc->get_vi().command(":prev");
  }
  
  m_lines_it--;

  m_stc->goto_line(*m_lines_it);
  
  return true;
}
