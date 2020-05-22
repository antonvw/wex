////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Common defs for all wex classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/defs.h>

namespace wex
{
  const int DEBUG_MAX_DEBUGS = 25;
  const int FIND_MAX_FINDS   = 25;
  const int VCS_MAX_COMMANDS = 25;
  const int VIEW_MAX_PANES   = 10;

  /// The maximal number of files and projects to be supported.
  const int NUMBER_RECENT_FILES      = 25;
  const int NUMBER_RECENT_PROJECTS   = 25;
  const int ID_RECENT_PROJECT_LOWEST = wxID_FILE1 + NUMBER_RECENT_FILES + 1;

  /// These are used as window event ID's. Therefore after the highest wxWidgets
  /// ID.
  enum window_id
  {
    ID_LOWEST = wxID_HIGHEST + 1,

    ID_ALL_LOWEST, // all ALL commands after this one
    ID_ALL_CLOSE,
    ID_ALL_CLOSE_OTHERS,
    ID_ALL_CONFIG_GET,
    ID_ALL_SAVE,
    ID_ALL_STC_SET_LEXER,
    ID_ALL_STC_SET_LEXER_THEME,
    ID_ALL_STC_SYNC,
    ID_ALL_HIGHEST, // and before this one

    ID_CLEAR_FILES,
    ID_CLEAR_FINDS,
    ID_CLEAR_PROJECTS,

    ID_DEBUG_EXIT,
    ID_DEBUG_STDIN,
    ID_DEBUG_STDOUT,

    ID_EDIT_DEBUG_FIRST,
    ID_EDIT_DEBUG_LAST = ID_EDIT_DEBUG_FIRST + DEBUG_MAX_DEBUGS,

    ID_EDIT_CONTROL_CHAR,
    ID_EDIT_DEBUG_VARIABLE,
    ID_EDIT_EOL_DOS,
    ID_EDIT_EOL_UNIX,
    ID_EDIT_EOL_MAC,
    ID_EDIT_FILE_ACTION,
    ID_EDIT_FIND_NEXT,
    ID_EDIT_FIND_PREVIOUS,
    ID_EDIT_OPEN,
    ID_EDIT_SELECT_NONE,
    ID_EDIT_SELECT_INVERT,

    ID_EDIT_VCS_LOWEST,
    ID_EDIT_VCS_HIGHEST = ID_EDIT_VCS_LOWEST + VCS_MAX_COMMANDS,

    ID_FIND_FIRST,
    ID_FIND_LAST = ID_FIND_FIRST + FIND_MAX_FINDS,

    ID_SHELL_APPEND,
    ID_SHELL_APPEND_ERROR,
    ID_SHELL_COMMAND,
    ID_SHELL_COMMAND_STOP,

    ID_TOOL_LOWEST,
    ID_TOOL_REPLACE,
    ID_TOOL_REPORT_FIRST, // after this the first report
    ID_TOOL_REPORT_FIND,
    ID_TOOL_REPORT_KEYWORD,
    ID_TOOL_REPORT_LAST,
    ID_TOOL_HIGHEST,

    ID_UPDATE_STATUS_BAR,

    ID_VCS_LOWEST,
    ID_VCS_HIGHEST = ID_VCS_LOWEST + VCS_MAX_COMMANDS,

    ID_VIEW_MENUBAR,
    ID_VIEW_STATUSBAR,
    ID_VIEW_TITLEBAR,

    ID_VIEW_LOWEST, // aui panes that can be toggled
    ID_VIEW_HIGHEST = ID_VIEW_LOWEST + VIEW_MAX_PANES,

    ID_HIGHEST
  };
}; // namespace wex
