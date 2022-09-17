////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.cpp
// Purpose:   Implementation of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>

#include "addressrange-mark.h"
#include "block-lines.h"

wex::addressrange_mark::addressrange_mark(
  const addressrange&     ar,
  const data::substitute& subs)
  : m_ar(ar)
  , m_type(get_type(subs))
  , m_ex(ar.get_ex())
  , m_stc(ar.get_ex()->get_stc())
  , m_undo(m_stc)
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

wex::addressrange_mark::mark_t
wex::addressrange_mark::get_type(const data::substitute& data) const
{
  if (data.is_global_command())
  {
    if (data.commands() == "d")
    {
      return data.is_inverse() ? MARK_GLOBAL_DELETE_INVERSE :
                                 MARK_GLOBAL_DELETE;
    }
  }

  return MARK_NORMAL;
}

bool wex::addressrange_mark::search(const data::substitute& data)
{
  return m_stc->SearchInTarget(data.pattern()) != -1 &&
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
    !m_ex->marker_add('$', end_line))
  {
    return false;
  }

  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_ex->marker_line('#')),
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  return true;
}

bool wex::addressrange_mark::update()
{
  int begin_pos;

  switch (m_type)
  {
    case MARK_GLOBAL_DELETE:
      begin_pos = m_stc->PositionFromLine(m_ex->marker_line('T'));
      break;

    case MARK_GLOBAL_DELETE_INVERSE:
      begin_pos = m_stc->GetLineEndPosition(m_ex->marker_line('T'));
      break;

    default:
      begin_pos = m_ar.data().is_global() ?
                    m_stc->GetTargetEnd() :
                    m_stc->GetLineEndPosition(m_ex->marker_line('T'));
  }

  m_stc->SetTargetRange(
    begin_pos,
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  return m_stc->GetTargetStart() < m_stc->GetTargetEnd();
}
