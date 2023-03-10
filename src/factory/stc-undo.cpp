////////////////////////////////////////////////////////////////////////////////
// Name:      stc-undo.cpp
// Purpose:   Implementation of class wex::stc_undo
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/stc-undo.h>

wex::stc_undo::stc_undo(wex::factory::stc* stc, undo_t type)
  : m_stc(stc)
  , m_type(type)
{
  if (m_type.test(UNDO_ACTION))
  {
    m_stc->BeginUndoAction();
  }

  if (m_type.test(UNDO_POS))
  {
    m_stc->position_save();
  }

  if (m_type.test(UNDO_SEL_NONE))
  {
    m_is_empty = m_stc->GetSelectionEmpty();
  }
}

wex::stc_undo::~stc_undo()
{
  if (m_type.test(UNDO_ACTION))
  {
    m_stc->EndUndoAction();
  }

  if (m_type.test(UNDO_POS))
  {
    m_stc->position_restore();
  }

  if (m_type.test(UNDO_SEL_NONE) && m_is_empty)
  {
    m_stc->SelectNone();
  }
}
