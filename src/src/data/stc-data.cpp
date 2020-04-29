////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.cpp
// Purpose:   Implementation of wex::stc_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/indicator.h>
#include <wex/stc-data.h>
#include <wex/stc.h>

wex::stc_data::stc_data(stc* stc)
  : m_stc(stc)
{
}

wex::stc_data::stc_data(stc* stc, const stc_data& r)
  : m_stc(stc)
{
  *this = r;
}

wex::stc_data::stc_data(control_data& data, stc* stc)
  : m_data(data)
  , m_stc(stc)
{
}

wex::stc_data::stc_data(window_data& data, stc* stc)
  : m_data(control_data().window(data))
  , m_stc(stc)
{
}

wex::stc_data& wex::stc_data::operator=(const stc_data& r)
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

wex::stc_data&
wex::stc_data::flags(window_t flags, control_data::action_t action)
{
  m_data.flags<flags.size()>(flags, m_win_flags, action);

  return *this;
}

wex::stc_data& wex::stc_data::indicator_no(indicator_t t)
{
  m_indicator_no = t;

  return *this;
}

bool wex::stc_data::inject() const
{
  if (m_stc == nullptr)
    return false;

  bool injected = m_data.inject(
    [&]() {
      // line
      if (m_data.line() > 0)
      {
        const auto line =
          (m_data.line() - 1 >= m_stc->GetLineCount() ?
             m_stc->GetLineCount() - 1 :
             m_data.line() - 1);

        m_stc->GotoLine(line);
        m_stc->EnsureVisible(line);
        m_stc->EnsureCaretVisible();
        m_stc->SetIndicatorCurrent(m_indicator_no);
        m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);

        m_stc->set_indicator(
          indicator(m_indicator_no),
          m_stc->PositionFromLine(line),
          m_data.col() > 0 ? m_stc->PositionFromLine(line) + m_data.col() - 1 :
                             m_stc->GetLineEndPosition(line));
      }
      else if (m_data.line() == DATA_NUMBER_NOT_SET)
      {
        return false;
      }
      else
      {
        m_stc->DocumentEnd();
      }
      return true;
    },
    [&]() {
      // col
      const int max =
        (m_data.line() > 0) ? m_stc->GetLineEndPosition(m_data.line() - 1) : 0;
      const int asked = m_stc->GetCurrentPos() + m_data.col() - 1;

      m_stc->SetCurrentPos(asked < max ? asked : max);

      // Reset selection, seems necessary.
      m_stc->SelectNone();
      return true;
    },
    [&]() {
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
      else if (m_data.line() == DATA_NUMBER_NOT_SET)
      {
        m_stc->find_next(m_data.find(), m_data.find_flags());
      }
      else
      {
        m_stc->find_next(m_data.find(), m_data.find_flags(), false);
      }
      return true;
    },
    [&]() {
      // command
      return m_stc->get_vi().command(m_data.command().command());
    });

  if (!m_data.window().name().empty())
  {
    m_stc->SetName(m_data.window().name());
  }

  if (
    m_win_flags[WIN_READ_ONLY] || (m_stc->get_filename().file_exists() &&
                                   m_stc->get_filename().is_readonly()))
  {
    m_stc->SetReadOnly(true);
    injected = true;
  }

  if (m_stc->get_hexmode().set(m_win_flags[WIN_HEX]))
  {
    injected = true;
  }

  if (m_event_data.synced())
  {
    injected = true;

    if (m_event_data.pos_at_end())
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
      m_event_data.synced() ? path::status_t().set(path::STAT_SYNC) :
                              path::status_t());

    if (!m_event_data.synced())
    {
      m_stc->SetFocus();
    }
  }

  return injected;
}

wex::stc_data& wex::stc_data::menu(menu_t flags, control_data::action_t action)
{
  m_data.flags<flags.size()>(flags, m_menu_flags, action);

  return *this;
}

void wex::stc_data::event_data::set(stc* s, bool synced)
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
  const bool is_log = (s->get_filename().extension().find(".log") == 0);
  m_synced          = synced;
  m_synced_log      = synced && is_log && s->GetTextLength() > 1024;
}
