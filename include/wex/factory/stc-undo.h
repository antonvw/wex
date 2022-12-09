////////////////////////////////////////////////////////////////////////////////
// Name:      stc-undo.h
// Purpose:   Declaration of class wex::stc_undo
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/stc.h>

namespace wex
{
/// Offers a simple class to enforce several undo like actions
/// on an stc component.
class stc_undo
{
public:
  /// Undo flags.
  enum
  {
    UNDO_ACTION = 0, ///< enforce Begin, End UndoAction
    UNDO_POS,        ///< enforce position_save, position_restore
    UNDO_SEL_NONE,   ///< enforce if no selection at cons, also at destruct
  };

  /// A typedef containing undo flags.
  typedef std::bitset<3> undo_t;

  /// Constructor, depending on the type will start an action.
  stc_undo(
    /// the stc component
    wex::factory::stc* stc,
    ///  the kind of undo
    undo_t type = UNDO_ACTION);

  /// Destructor, will restore the actions taken.
  ~stc_undo();

private:
  wex::factory::stc* m_stc;
  const undo_t       m_type;
  bool               m_is_empty{false};
};
}; // namespace wex
