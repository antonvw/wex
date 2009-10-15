/******************************************************************************\
* File:          defs.h
* Purpose:       Common definitions for wxExtension report
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/
#ifndef _EX_REPORT_DEFS_H
#define _EX_REPORT_DEFS_H

#include <wx/extension/defs.h>

// The maximal number of files and projects to be supported.
#define NUMBER_RECENT_FILES 25
#define NUMBER_RECENT_PROJECTS 25

// Commands supported by wxExtension report.
enum
{
  ID_EXTENSION_REPORT_LOWEST = ID_TOOL_FIRST_USER + 1,

  ID_TOOL_SQL,
  ID_TOOL_REPORT_SQL = ID_TOOL_REPORT_FIRST_USER + 1,
  ID_FIND_TEXT,
  ID_MATCH_WHOLE_WORD,
  ID_MATCH_CASE,
  ID_REGULAR_EXPRESSION,
  ID_LIST_LOWEST = ID_TOOL_HIGHEST + 1,
  ID_LIST_COMPARE,
  ID_LIST_COMPARELAST,
  ID_LIST_OPEN_ITEM,
  ID_LIST_RUN_MAKE,
  ID_LIST_VERSIONLIST,
  ID_LIST_HIGHEST,
  ID_LIST_SEND_ITEM, // only for wxExListViewFile
  ID_LIST_ALL_ITEMS, // not a real command, used by exForEach
  ID_LIST_ALL_CLOSE, // not a real command, used by exForEach
  ID_PROJECT_SAVE,
  ID_RECENT_LOWEST,
  ID_RECENT_FILE_LOWEST,
  ID_RECENT_FILE_HIGHEST = ID_RECENT_FILE_LOWEST + NUMBER_RECENT_FILES,
  ID_RECENT_PROJECT_LOWEST,
  ID_RECENT_PROJECT_HIGHEST = ID_RECENT_PROJECT_LOWEST + NUMBER_RECENT_PROJECTS,
  ID_RECENT_HIGHEST,
  ID_SPECIAL_FIND_IN_FILES,
  ID_SPECIAL_REPLACE_IN_FILES,
  ID_STC_LOWEST,
  ID_STC_ADD_HEADER,
  ID_STC_FIND_FILES,
  ID_STC_REPLACE_FILES,
  ID_STC_COMPARE,
  ID_STC_HIGHEST,
  ID_TERMINATED_PROCESS,
  ID_VIEW_STATUSBAR,
  ID_VIEW_TOOLBAR,

  ID_EXTENSION_REPORT_HIGHEST,
};

#endif
