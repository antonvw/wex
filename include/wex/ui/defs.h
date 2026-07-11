////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Defs for all wex ui classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/defs.h>

/*! \file */

namespace wex
{
/// These are used as window ui event ID's, and placed on the ID_UI_ range.
enum window_ui_id
{
  ID_CLEAR_DIFFS = ID_UI_LOWEST,
  ID_CLEAR_FILES,
  ID_CLEAR_FINDS,
  ID_CLEAR_PROJECTS,

  ID_EDIT_CONTROL_CHAR,
  ID_EDIT_DEBUG_VARIABLE,
  ID_EDIT_FILE_ACTION,
  ID_EDIT_FIND_NEXT,
  ID_EDIT_FIND_PREVIOUS,
  ID_EDIT_OPEN,
  ID_EDIT_REV_COMPARE,
  ID_EDIT_REV_OPEN,
  ID_EDIT_SELECT_NONE,
  ID_EDIT_SELECT_INVERT,

  ID_LIST_COMPARE,

  ID_LSP_CODE_COMPLETION,
  ID_LSP_CROSS_REFERENCES,

  ID_LSP_DEFINITION,
  ID_LSP_IMPLEMENTATION,

  ID_LSP_DIAGNOSTICS,
  ID_LSP_HOVER,
  ID_LSP_IWYU,
  ID_LSP_RENAME,
  ID_LSP_SHOW_MESSAGE,
  ID_LSP_SIGNATURE_HELP,

  ID_UPDATE_STATUS_BAR,

  ID_VIEW_MENUBAR,
  ID_VIEW_STATUSBAR,
  ID_VIEW_TITLEBAR,

  ID_VIEW_LOWEST, // aui panes that can be toggled
  ID_VIEW_HIGHEST = ID_VIEW_LOWEST + VIEW_MAX_PANES,
};
}; // namespace wex
