////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diffs.cpp
// Purpose:   Implementation of class unified_diffs
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <numeric>

#include <wex/core/log.h>
#include <wex/factory/frame.h>
#include <wex/factory/stc.h>
#include <wex/factory/unified-diffs.h>
#include <wx/app.h>

wex::unified_diffs::unified_diffs(factory::stc* s)
  : m_stc(s)
  , m_lines_it(m_lines.begin())
{
}

bool wex::unified_diffs::checkout(size_t line)
{
  if (auto it = m_lines.find(line); it != m_lines.end())
  {
    if (it->second.range_from_count() > 0)
    {
      m_stc->insert_text(
        m_stc->GetLineEndPosition(line - 1),
        "\n" + boost::join(it->second.text_removed(), "\n"));
    }

    if (it->second.range_to_count() > 0)
    {
      m_stc->DeleteRange(
        m_stc->GetLineEndPosition(line - 1),
        m_stc->GetLineEndPosition(line + it->second.range_to_count()));
    }

    m_lines.erase(line);
    m_lines_it = m_lines.begin();
    return true;
  }

  return false;
}

void wex::unified_diffs::clear()
{
  m_lines.clear();
  m_lines_it = m_lines.begin();
}

bool wex::unified_diffs::end()
{
  if (m_lines.empty())
  {
    return false;
  }

  m_lines_it = std::prev(m_lines.end());

  m_stc->goto_line(m_lines_it->first);

  return true;
}

bool wex::unified_diffs::first()
{
  if (m_lines.empty())
  {
    return false;
  }

  m_lines_it = m_lines.begin();

  m_stc->goto_line(m_lines_it->first);

  return true;
}

void wex::unified_diffs::insert(const factory::unified_diff* diff)
{
  if (diff->range_from_start() > 0 && diff->range_from_count() > 0)
  {
    m_lines[diff->range_from_start() - 1] = *diff;
  }

  if (diff->range_to_start() > 0 && diff->range_to_count() > 0)
  {
    m_lines[diff->range_to_start() - 1] = *diff;
  }

  m_lines_it = m_lines.begin();
}

bool wex::unified_diffs::next()
{
  if (m_lines.empty())
  {
    return false;
  }

  if (m_stc->GetCurrentPos() == 0)
  {
    return first();
  }

  if (m_lines_it == m_lines.end() || m_lines_it == std::prev(m_lines.end()))
  {
    if (auto* frame = dynamic_cast<factory::frame*>(wxTheApp->GetTopWindow());
        frame != nullptr)
    {
      frame->page_next(true);
    }

    return false;
  }

  m_lines_it++;

  m_stc->goto_line(m_lines_it->first);

  return true;
}

size_t wex::unified_diffs::pos() const
{
  return m_lines.begin() != m_lines.end() ?
           std::distance(m_lines.begin(), m_lines_it) + 1 :
           0;
}

bool wex::unified_diffs::prev()
{
  if (m_lines.empty())
  {
    return false;
  }

  if (m_stc->GetCurrentPos() >= m_stc->GetLength() - 2)
  {
    return end();
  }

  if (m_lines_it == m_lines.begin())
  {
    if (auto* frame = dynamic_cast<factory::frame*>(wxTheApp->GetTopWindow());
        frame != nullptr)
    {
      frame->page_prev(true);
    }

    return false;
  }

  m_lines_it--;

  m_stc->goto_line(m_lines_it->first);

  return true;
}

void wex::unified_diffs::status() const
{
  if (m_lines.empty())
  {
    log::status("no differences present");
  }
  else
  {
    log::status("diff") << pos() << "from" << size();
  }
}
