/******************************************************************************\
* File:          defs.h
* Purpose:       Common defs for all wxExtension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXDEFS_H
#define _EXDEFS_H

/// These are used as window event ID's. Therefore after the highest wxWidgets ID.
enum wxExId
{
  ID_EDIT_LOWEST = wxID_HIGHEST + 1,

  ID_ALL_LOWEST,         // all ALL commands after this one
  ID_ALL_STC_CLOSE,
  ID_ALL_STC_COLOURISE,
  ID_ALL_STC_CONFIG_GET,
  ID_ALL_STC_SAVE,
  ID_ALL_STC_SET_LEXER,
  ID_ALL_HIGHEST,        // and before this one

  ID_SHELL_COMMAND,
  ID_SHELL_COMMAND_STOP,

  ID_EDIT_FIND_NEXT,
  ID_EDIT_FIND_PREVIOUS,

  ID_EDIT_STC_LOWEST,    // all STC commands after this one
  ID_EDIT_SELECT_NONE,
  ID_EDIT_SELECT_INVERT,
  ID_EDIT_CONTROL_CHAR,
  ID_EDIT_HEX_DEC_CALLTIP,
  ID_EDIT_UPPERCASE,
  ID_EDIT_LOWERCASE,
  ID_EDIT_MACRO_PLAYBACK,
  ID_EDIT_MACRO_START_RECORD,
  ID_EDIT_MACRO_STOP_RECORD,
  ID_EDIT_OPEN_LINK,
  ID_EDIT_OPEN_BROWSER,
  ID_EDIT_SORT_TOGGLE,
  ID_EDIT_INSERT_DATE,
  ID_EDIT_INSERT_SEQUENCE,
  ID_EDIT_TOGGLE_FOLD,
  ID_EDIT_FOLD_ALL,
  ID_EDIT_UNFOLD_ALL,
  ID_EDIT_STATUS_BAR,
  ID_EDIT_EOL_DOS,
  ID_EDIT_EOL_UNIX,
  ID_EDIT_EOL_MAC,
  ID_EDIT_SVN_LOWEST,
  ID_EDIT_SVN_ADD,
  ID_EDIT_SVN_BLAME,
  ID_EDIT_SVN_CAT,
  ID_EDIT_SVN_COMMIT,
  ID_EDIT_SVN_DIFF,
  ID_EDIT_SVN_HELP,
  ID_EDIT_SVN_INFO,
  ID_EDIT_SVN_LOG,
  ID_EDIT_SVN_PROPLIST,
  ID_EDIT_SVN_PROPSET,
  ID_EDIT_SVN_REVERT,
  ID_EDIT_SVN_STAT,
  ID_EDIT_SVN_UPDATE,
  ID_EDIT_SVN_HIGHEST,
  ID_EDIT_STC_HIGHEST,

  ID_EDIT_HIGHEST,
};

/// The available tools.
/// These are also used as window event ID's. Therefore after the highest edit ID.
enum wxExToolId
{
  ID_TOOL_LOWEST = ID_EDIT_HIGHEST + 1,
  ID_TOOL_LINE,          // lines with code or empty lines
  ID_TOOL_LINE_COMMENT,  // only comments
  ID_TOOL_LINE_CODE,     // only lines with code
  ID_TOOL_REVISION_RECENT,
  ID_TOOL_FIRST_USER,    // after this your own tool
  ID_TOOL_REPORT_FIRST = ID_TOOL_FIRST_USER + 100,  // after this the first report
  ID_TOOL_REPORT_FIND,
  ID_TOOL_REPORT_REPLACE,
  ID_TOOL_REPORT_COUNT,
  ID_TOOL_REPORT_VERSION,
  ID_TOOL_REPORT_REVISION,
  ID_TOOL_REPORT_KEYWORD,
  ID_TOOL_REPORT_FIRST_USER, // after this your own report
  ID_TOOL_REPORT_LAST = ID_TOOL_REPORT_FIRST_USER + 100,   // before this is the last report
  ID_TOOL_HIGHEST,
};
#endif
