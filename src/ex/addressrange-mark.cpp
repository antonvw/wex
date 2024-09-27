////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.cpp
// Purpose:   Implementation of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>

#include "addressrange-mark.h"
#include "block-lines.h"

wex::addressrange_mark::addressrange_mark(
  const addressrange&     ar,
  const data::substitute& subs)
  : m_ar(ar)
  , m_ex(ar.get_ex())
  , m_stc(ar.get_ex()->get_stc())
  , m_undo(m_stc)
  , m_data(subs)
{
  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
}

wex::addressrange_mark::~addressrange_mark()
{
  m_ex->marker_delete('#');
  m_ex->marker_delete('$');
  m_ex->marker_delete('T');
}

void wex::addressrange_mark::end(bool indicator_clear)
{
  if (m_stc->is_hexmode())
  {
    m_stc->get_hexmode_sync();
  }

  if (indicator_clear)
  {
    m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
  }

  if (m_ar.is_selection())
  {
    m_stc->SetSelection(
      m_stc->PositionFromLine(m_ex->marker_line('#')),
      m_stc->PositionFromLine(m_ex->marker_line('$') + m_corrected));
  }
}

wex::block_lines wex::addressrange_mark::get_block_lines() const
{
  return block_lines(m_ex).single();
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

bool wex::addressrange_mark::search()
{
  if (m_data.pattern().empty())
  {
    return false;
  }
  else if (m_data.pattern() == "$")
  {
    if (m_ex->marker_line('T') == m_ex->marker_line('$'))
    {
      return false;
    }

    m_stc->SetTargetRange(
      m_stc->GetLineEndPosition(m_ex->marker_line('T')),
      m_stc->GetLineEndPosition(m_ex->marker_line('T')));

    return m_ex->marker_add(
      'T',
      m_stc->LineFromPosition(m_stc->GetTargetEnd()) + 1);
  }

  return m_stc->SearchInTarget(m_data.pattern()) != -1 &&
         m_ex->marker_add('T', m_stc->LineFromPosition(m_stc->GetTargetEnd()));
}

bool wex::addressrange_mark::set()
{
  auto end_line = m_ar.end().get_line() - 1;

  if (
    !m_stc->GetSelectedText().empty() &&
    m_stc->GetLineSelEndPosition(end_line) == m_stc->PositionFromLine(end_line))
  {
    end_line--;
    m_corrected = 1;
  }

  if (
    !m_ex->marker_add('#', m_ar.begin().get_line() - 1) ||
    !m_ex->marker_add('T', m_ar.begin().get_line() - 1) ||
    !m_ex->marker_add('$', end_line))
  {
    return false;
  }

  set_target(m_stc->PositionFromLine(m_ex->marker_line('#')));

  m_last_range_line = false;

  return true;
}

void wex::addressrange_mark::set_target(int start)
{
  m_stc->SetTargetRange(
    start,
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  log::trace("addressrange_mark set_target")
    << m_stc->GetTargetStart() << "," << m_stc->GetTargetEnd()
    << "T:" << m_ex->marker_line('T') << "$:" << m_ex->marker_line('$');
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
          m_stc->GetLineEndPosition(m_ex->marker_line('T') + lines_changed);
      break;

    case mark_t::GLOBAL_CHANGE:
      target_start += m_ar.data().is_global() ?
                        m_stc->GetTargetEnd() :
                        m_stc->GetLineEndPosition(m_ex->marker_line('T') - 1);
      break;

    case mark_t::GLOBAL_DELETE:
      target_start += m_stc->PositionFromLine(m_ex->marker_line('T'));
      break;

    case mark_t::GLOBAL_DELETE_INVERSE:
      target_start += m_stc->GetLineEndPosition(m_ex->marker_line('T'));
      break;

    default:
      target_start += m_ar.data().is_global() ?
                        m_stc->GetTargetEnd() :
                        m_stc->GetLineEndPosition(m_ex->marker_line('T'));
  }

  set_target(target_start);

  if (
    m_ex->marker_line('T') == m_ex->marker_line('$') &&
    !m_ar.data().is_global())
  {
    if (m_last_range_line)
    {
      return false;
    }

    m_last_range_line = true;
  }

  return m_stc->GetTargetStart() <= m_stc->GetTargetEnd();
}
