////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Common defs for all wxExtension classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXDEFS_H
#define _EXDEFS_H

const int TOOL_MAX = 25;
const int TOOL_MAX_REPORTS = 10;
const int VCS_MAX_COMMANDS = 25;

/// These are used as window event ID's. Therefore after the highest wxWidgets ID.
enum wxExId
{
  ID_EDIT_LOWEST = wxID_HIGHEST + 1,

  ID_ALL_LOWEST,         // all ALL commands after this one
  ID_ALL_STC_CLOSE,
  ID_ALL_STC_CONFIG_GET,
  ID_ALL_STC_SAVE,
  ID_ALL_STC_SET_LEXER,
  ID_ALL_STC_SET_LEXER_THEME,
  ID_ALL_HIGHEST,        // and before this one

  ID_SHELL_COMMAND,
  ID_SHELL_COMMAND_STOP,

  ID_EDIT_FIND_NEXT,
  ID_EDIT_FIND_PREVIOUS,

  ID_EDIT_NEXT,
  ID_EDIT_PREVIOUS,

  ID_EDIT_SELECT_NONE,
  ID_EDIT_SELECT_INVERT,

  // Here are the STC file commands,
  // add them to stc file event macro as well.
  ID_EDIT_OPEN_LINK,
  ID_EDIT_OPEN_BROWSER,
  ID_EDIT_SHOW_PROPERTIES,

  ID_VIEW_MENUBAR,
  ID_VIEW_STATUSBAR,
  ID_VIEW_TITLEBAR,

  ID_VIEW_LOWEST,        // aui panes that can be toggled
  ID_VIEW_FINDBAR,
  ID_VIEW_TOOLBAR,
  ID_VIEW_HIGHEST,

  ID_EDIT_STC_LOWEST,    // all STC commands after this one
  ID_EDIT_READ,
  ID_EDIT_CONTROL_CHAR,
  ID_EDIT_HEX_DEC_CALLTIP,
  ID_EDIT_UPPERCASE,
  ID_EDIT_LOWERCASE,
  ID_EDIT_SORT_TOGGLE,
  ID_EDIT_TOGGLE_FOLD,
  ID_EDIT_FOLD_ALL,
  ID_EDIT_UNFOLD_ALL,
  ID_EDIT_EOL_DOS,
  ID_EDIT_EOL_UNIX,
  ID_EDIT_EOL_MAC,
  ID_EDIT_HEX,
  ID_EDIT_MARKER_NEXT,
  ID_EDIT_MARKER_PREVIOUS,
  ID_EDIT_ZOOM_IN,
  ID_EDIT_ZOOM_OUT,
  ID_EDIT_STC_HIGHEST,   // and before this one

  ID_EDIT_VCS_LOWEST,
  ID_EDIT_VCS_HIGHEST = ID_EDIT_VCS_LOWEST + VCS_MAX_COMMANDS,

  ID_VCS_LOWEST,
  ID_VCS_HIGHEST = ID_VCS_LOWEST + ID_EDIT_VCS_HIGHEST - ID_EDIT_VCS_LOWEST,

  ID_EDIT_HIGHEST
};

/// The available tools.
/// These are also used as window event ID's. Therefore after the highest edit ID.
enum wxExToolId
{
  ID_TOOL_LOWEST = ID_EDIT_HIGHEST + 1,
  ID_TOOL_FIRST_USER,    // after this your own tool
  ID_TOOL_REPORT_FIRST = ID_TOOL_FIRST_USER + TOOL_MAX,  // after this the first report
  ID_TOOL_REPORT_FIND,
  ID_TOOL_REPORT_REPLACE,
  ID_TOOL_REPORT_KEYWORD,
  ID_TOOL_REPORT_FIRST_USER, // after this your own report
  ID_TOOL_REPORT_LAST = ID_TOOL_REPORT_FIRST_USER + TOOL_MAX_REPORTS,  // before this is the last report
  ID_TOOL_HIGHEST
};
#endif
