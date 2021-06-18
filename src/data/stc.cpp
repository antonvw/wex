////////////////////////////////////////////////////////////////////////////////
// Name:      data/stc.cpp
// Purpose:   Implementation of wex::data::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/data/stc.h>
#include <wex/factory/stc.h>
#include <wex/indicator.h>
#include <wex/path.h>

wex::data::stc::stc(wex::factory::stc* stc)
  : m_stc(stc)
{
}

wex::data::stc::stc(wex::factory::stc* stc, const data::stc& r)
  : m_stc(stc)
{
  *this = r;
}

wex::data::stc::stc(data::control& data, wex::factory::stc* stc)
  : m_data(data)
  , m_stc(stc)
{
}

wex::data::stc::stc(data::window& data, wex::factory::stc* stc)
  : m_data(data::control().window(data))
  , m_stc(stc)
{
}

wex::data::stc& wex::data::stc::operator=(const data::stc& r)
{
  if (this != &r)
  {
    m_indicator_no = r.m_indicator_no;
    m_data         = r.m_data;
    m_menu_flags   = r.m_menu_flags;
    m_win_flags    = r.m_win_flags;
    m_event_data   = r.m_event_data;

    if (m_stc != nullptr && r.m_stc != nullptr)
    {
      m_stc = r.m_stc;
    }
  }

  return *this;
}

wex::data::stc&
wex::data::stc::flags(window_t flags, data::control::action_t action)
{
  m_data.flags<flags.size()>(flags, m_win_flags, action);

  return *this;
}

wex::data::stc& wex::data::stc::indicator_no(indicator_t t)
{
  m_indicator_no = t;

  return *this;
}

bool wex::data::stc::inject() const
{
  if (m_stc == nullptr)
    return false;

  bool injected = m_data.inject(
    [&]()
    {
      // line
      if (m_data.line() > 0)
      {
        int line;

        if (m_stc->get_line_count() == LINE_COUNT_UNKNOWN)
        {
          line = m_data.line() - 1;
        }
        else if (m_data.line() - 1 >= m_stc->get_line_count())
        {
          line = m_stc->get_line_count() - 1;
        }
        else
        {
          line = m_data.line() - 1;
        }

        m_stc->goto_line(line);

        if (m_stc->is_visual())
        {
          m_stc->set_indicator(
            indicator(m_indicator_no),
            std::max(m_stc->PositionFromLine(line), 0),
            m_data.col() > 0 ?
              m_stc->PositionFromLine(line) + m_data.col() - 1 :
              m_stc->GetLineEndPosition(line));
        }
      }
      else if (m_data.line() == NUMBER_NOT_SET)
      {
        return false;
      }
      else
      {
        m_stc->DocumentEnd();
      }
      return true;
    },
    [&]()
    {
      // col
      const int max =
        (m_data.line() > 0) ? m_stc->GetLineEndPosition(m_data.line() - 1) : 0;
      const int asked = m_stc->GetCurrentPos() + m_data.col() - 1;

      m_stc->SetCurrentPos(asked < max ? asked : max);

      return true;
    },
    [&]()
    {
      // find
      if (m_data.line() > 0)
      {
        const int start_pos = m_stc->PositionFromLine(m_data.line() - 1);
        const int end_pos   = m_stc->GetLineEndPosition(m_data.line() - 1);

        m_stc->set_search_flags(-1);
        m_stc->SetTargetRange(start_pos, end_pos);

        if (m_stc->SearchInTarget(m_data.find()) != -1)
        {
          m_stc->SetSelection(m_stc->GetTargetStart(), m_stc->GetTargetEnd());
        }
      }
      else if (m_data.line() == NUMBER_NOT_SET)
      {
        m_stc->find(m_data.find(), m_data.find_flags());
      }
      else
      {
        m_stc->find(m_data.find(), m_data.find_flags(), false);
      }
      return true;
    },
    [&]()
    {
      // command
      return m_stc->vi_command(m_data.command());
    });

  if (!m_data.window().name().empty())
  {
    m_stc->SetName(m_data.window().name());
  }

  if (
    m_win_flags[WIN_READ_ONLY] ||
    (m_stc->path().file_exists() && m_stc->path().is_readonly()))
  {
    m_stc->SetReadOnly(true);
    injected = true;
  }

  if (config(_("stc.Ex mode show hex")).get(false) && !m_stc->is_visual())
  {
    if (m_stc->set_hexmode(true))
    {
      injected = true;
    }
  }
  else if (m_stc->set_hexmode(m_win_flags[WIN_HEX]))
  {
    injected = true;
  }

  if (m_event_data.is_synced())
  {
    injected = true;

    if (m_event_data.is_pos_at_end())
    {
      m_stc->DocumentEnd();
    }
  }

  if (m_event_data.pos_start() != m_event_data.pos_end())
  {
    m_stc->SetSelection(m_event_data.pos_start(), m_event_data.pos_end());
    injected = true;
  }

  if (injected)
  {
    m_stc->properties_message(
      m_event_data.is_synced() ? path::status_t().set(path::STAT_SYNC) :
                                 path::status_t());

    if (!m_event_data.is_synced() && m_stc->is_visual())
    {
      m_stc->SetFocus();
    }
  }

  return injected;
}

wex::data::stc&
wex::data::stc::menu(menu_t flags, data::control::action_t action)
{
  m_data.flags<flags.size()>(flags, m_menu_flags, action);

  return *this;
}

void wex::data::stc::event_data::set(factory::stc* s, bool synced)
{
  if (s == nullptr)
  {
    return;
  }

  m_pos_at_end = (s->GetCurrentPos() >= s->GetTextLength() - 1);

  if (!s->GetSelectedText().empty())
  {
    s->GetSelection(&m_pos_start, &m_pos_end);
  }
  else
  {
    m_pos_start = -1;
    m_pos_end   = -1;
  }

  // Synchronizing by appending only new data only works for log files.
  // Other kind of files might get new data anywhere inside the file,
  // we cannot sync that by keeping pos.
  // Also only do it for reasonably large files.
  const bool is_log = (s->path().extension().starts_with(".log"));
  m_synced          = synced;
  m_synced_log      = synced && is_log && s->GetTextLength() > 1024;
}
