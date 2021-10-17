////////////////////////////////////////////////////////////////////////////////
// Name:      global-env.cpp
// Purpose:   Implementation of class wex::global_env
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/ex.h>

#include "global-env.h"

wex::global_env::global_env(const addressrange* ar)
  : m_ex(ar->m_ex)
  , m_ar(ar)
  , m_stc(m_ex->get_stc())
{
  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
  m_stc->set_search_flags(m_ex->search_flags());
  m_stc->BeginUndoAction();
  m_ex->marker_add('%', m_ar->get_end().get_line() - 1);

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         addressrange::data().commands(),
         boost::char_separator<char>("|")))
  {
    // Prevent recursive global.
    if (it[0] != 'g' && it[0] != 'v')
    {
      if (it[0] == 'd' || it[0] == 'm')
      {
        m_changes++;
      }

      m_commands.emplace_back(it);
    }
  }
}

wex::global_env::~global_env()
{
  m_stc->EndUndoAction();
  m_ex->marker_delete('%');
}

bool wex::global_env::for_each(int line) const
{
  if (!has_commands())
  {
    m_stc->set_indicator(
      m_ar->m_find_indicator,
      m_stc->GetTargetStart(),
      m_stc->GetTargetEnd());
    return true;
  }
  else
  {
    return std::all_of(
      m_commands.begin(),
      m_commands.end(),
      [this, line](const std::string& it)
      {
        if (!m_ex->command(":" + std::to_string(line + 1) + it))
        {
          m_ex->frame()->show_ex_message(
            m_ex->get_command().command() + " failed");
          return false;
        }
        return true;
      });
  }
}

bool wex::global_env::for_each(int start, int& end, int& hits) const
{
  if (start < end)
  {
    for (int i = start; i < end && i < m_stc->get_line_count() - 1;)
    {
      if (has_commands())
      {
        if (!for_each(i))
          return false;
      }
      else
      {
        m_stc->set_indicator(
          m_ar->m_find_indicator,
          m_stc->PositionFromLine(i),
          m_stc->GetLineEndPosition(i));
      }

      if (m_changes == 0)
      {
        i++;
      }
      else
      {
        end -= m_changes;
      }

      hits++;
    }
  }
  else
  {
    end++;
  }

  return true;
}

bool wex::global_env::global(const data::substitute& data)
{
  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_ar->get_begin().get_line() - 1),
    m_stc->GetLineEndPosition(m_ex->marker_line('%')));

  const bool infinite =
    (m_changes > 0 && data.commands() != "$" && data.commands() != "1" &&
     data.commands() != "d");

  int start = 0;

  while (m_stc->SearchInTarget(data.pattern()) != -1)
  {
    auto match = m_stc->LineFromPosition(m_stc->GetTargetStart());

    if (!data.is_inverse())
    {
      if (!for_each(match))
        return false;
      m_hits++;
    }
    else
    {
      if (!for_each(start, match, m_hits))
        return false;
      start = match + 1;
    }

    if (m_hits > 50 && infinite)
    {
      m_ex->frame()->show_ex_message(
        "possible infinite loop at " + std::to_string(match));
      return false;
    }

    m_stc->SetTargetRange(
      m_changes > 0 ? m_stc->PositionFromLine(match) : m_stc->GetTargetEnd(),
      m_stc->GetLineEndPosition(m_ex->marker_line('%')));

    if (m_stc->GetTargetStart() >= m_stc->GetTargetEnd())
    {
      break;
    }
  }

  if (data.is_inverse())
  {
    if (auto match = m_stc->get_line_count(); !for_each(start, match, m_hits))
    {
      return false;
    }
  }

  return true;
}
