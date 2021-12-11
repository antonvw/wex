////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange-mark.cpp
// Purpose:   Implementation of class wex::addressrange_mark
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/stc.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/ex.h>

#include "addressrange-mark.h"
#include "block-lines.h"

wex::addressrange_mark::addressrange_mark(const addressrange& ar)
  : m_ar(ar)
  , m_ex(ar.get_ex())
  , m_stc(ar.get_ex()->get_stc())
{
  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
}

wex::addressrange_mark::~addressrange_mark()
{
  m_ex->marker_delete('#');
  m_ex->marker_delete('$');
}

void wex::addressrange_mark::end(bool indicator_clear)
{
  if (m_stc->is_hexmode())
  {
    m_stc->get_hexmode_sync();
  }

  m_stc->EndUndoAction();

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

bool wex::addressrange_mark::set()
{
  auto end_line = m_ar.get_end().get_line() - 1;

  if (
    !m_stc->GetSelectedText().empty() &&
    m_stc->GetLineSelEndPosition(end_line) == m_stc->PositionFromLine(end_line))
  {
    end_line--;
    m_corrected = 1;
  }

  if (
    !m_ex->marker_add('#', m_ar.get_begin().get_line() - 1) ||
    !m_ex->marker_add('$', end_line))
  {
    return false;
  }

  m_stc->BeginUndoAction();

  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_ex->marker_line('#')),
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  return true;
}

bool wex::addressrange_mark::update(int begin_pos)
{
  auto use_pos = begin_pos;

  if (use_pos == -1)
  {
    use_pos = m_ar.data().is_global() ?
                m_stc->GetTargetEnd() :
                m_stc->GetLineEndPosition(
                  m_stc->LineFromPosition(m_stc->GetTargetEnd()));
  }

  m_stc->SetTargetRange(
    use_pos,
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  if (m_stc->GetTargetStart() >= m_stc->GetTargetEnd())
  {
    return false;
  }

  return true;
}
