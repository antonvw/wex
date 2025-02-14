////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.cpp
// Purpose:   Implementation of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>

#include "addressrange-mark.h"
#include "block-lines.h"

wex::addressrange_mark::addressrange_mark(
  const addressrange&     ar,
  const data::substitute& subs,
  bool                    global)
  : m_ar(ar)
  , m_ex(ar.get_ex())
  , m_stc(ar.get_ex()->get_stc())
  , m_undo(m_stc)
  , m_data(subs)
  , ma_b(global ? 'x' : '#')
  , ma_t(global ? 'y' : 'T')
  , ma_e(global ? 'z' : '$')
{
}

wex::addressrange_mark::~addressrange_mark()
{
  m_ex->marker_delete(ma_b);
  m_ex->marker_delete(ma_e);
  m_ex->marker_delete(ma_t);
}

void wex::addressrange_mark::end(bool indicator_clear)
{
  if (m_stc->is_hexmode())
  {
    m_stc->get_hexmode_sync();
  }

  if (indicator_clear)
  {
    m_stc->IndicatorClearRange(
      m_stc->PositionFromLine(marker_begin()),
      m_stc->PositionFromLine(marker_end() + m_corrected));
  }

  if (m_ar.is_selection())
  {
    m_stc->SetSelection(
      m_stc->PositionFromLine(marker_begin()),
      m_stc->PositionFromLine(marker_end() + m_corrected));
  }
}

wex::addressrange_mark::mark_t wex::addressrange_mark::get_type() const
{
  if (m_data.is_global_command())
  {
    if (m_data.commands().starts_with("a"))
    {
      return mark_t::GLOBAL_APPEND;
    }

    if (m_data.commands() == "d")
    {
      return m_data.is_inverse() ? mark_t::GLOBAL_DELETE_INVERSE :
                                   mark_t::GLOBAL_DELETE;
    }

    if (m_data.commands().starts_with("c"))
    {
      return mark_t::GLOBAL_CHANGE;
    }
  }

  return mark_t::NORMAL;
}

int wex::addressrange_mark::marker_begin() const
{
  return m_ex->marker_line(ma_b);
}

int wex::addressrange_mark::marker_end() const
{
  return m_ex->marker_line(ma_e);
}

int wex::addressrange_mark::marker_target() const
{
  return m_ex->marker_line(ma_t);
}

bool wex::addressrange_mark::search()
{
  if (m_data.pattern().empty())
  {
    return false;
  }

  if (m_data.pattern() == "$")
  {
    if (marker_target() == marker_end())
    {
      return false;
    }

    m_stc->SetTargetRange(
      m_stc->GetLineEndPosition(marker_target()),
      m_stc->GetLineEndPosition(marker_target()));

    return m_ex->marker_add(
      ma_t,
      m_stc->LineFromPosition(m_stc->GetTargetEnd()) + 1);
  }

  return m_stc->SearchInTarget(m_data.pattern()) != -1 &&
         m_ex->marker_add(ma_t, m_stc->LineFromPosition(m_stc->GetTargetEnd()));
}

bool wex::addressrange_mark::set()
{
  log::trace("addressrange_mark set");

  auto end_line = m_ar.end().get_line() - 1;

  if (
    !m_stc->GetSelectedText().empty() &&
    m_stc->GetLineSelEndPosition(end_line) == m_stc->PositionFromLine(end_line))
  {
    end_line--;
    m_corrected = 1;
  }

  if (
    !m_ex->marker_add(ma_b, m_ar.begin().get_line() - 1) ||
    !m_ex->marker_add(ma_t, m_ar.begin().get_line() - 1) ||
    !m_ex->marker_add(ma_e, end_line))
  {
    return false;
  }

  set_target(m_stc->PositionFromLine(marker_begin()));

  m_stc->IndicatorClearRange(
    m_stc->PositionFromLine(marker_begin()),
    m_stc->PositionFromLine(marker_end() + m_corrected));

  m_last_range_line = false;

  return true;
}

void wex::addressrange_mark::set_target(int start)
{
  m_stc->SetTargetRange(start, m_stc->GetLineEndPosition(marker_end()));

  log::trace("addressrange_mark set_target")
    << m_stc->GetTargetStart() << "," << m_stc->GetTargetEnd() << ma_t
    << marker_target() << ma_e << marker_end();
}

bool wex::addressrange_mark::update(int lines_changed)
{
  int target_start = 0;

  if (m_stc->GetTargetStart() == m_stc->GetTargetEnd())
  {
    target_start++; // prevent looping on matching empty line
  }

  switch (get_type())
  {
    case mark_t::GLOBAL_APPEND:
      target_start +=
        m_ar.data().is_global() ?
          m_stc->GetTargetEnd() :
          m_stc->GetLineEndPosition(marker_target() + lines_changed);
      break;

    case mark_t::GLOBAL_CHANGE:
      target_start += m_ar.data().is_global() ?
                        m_stc->GetTargetEnd() :
                        m_stc->GetLineEndPosition(marker_target() - 1);
      break;

    case mark_t::GLOBAL_DELETE:
      target_start += m_stc->PositionFromLine(marker_target());
      break;

    case mark_t::GLOBAL_DELETE_INVERSE:
      target_start += m_stc->GetLineEndPosition(marker_target());
      break;

    default:
      target_start += m_ar.data().is_global() ?
                        m_stc->GetTargetEnd() :
                        m_stc->GetLineEndPosition(marker_target());
  }

  set_target(target_start);

  if (marker_target() == marker_end() && !m_ar.data().is_global())
  {
    if (m_last_range_line)
    {
      return false;
    }

    m_last_range_line = true;
  }

  return m_stc->GetTargetStart() <= m_stc->GetTargetEnd();
}
