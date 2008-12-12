/******************************************************************************\
* File:          defs.h
* Purpose:       Common definitions for wxfiletool
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/
#ifndef _FTDEFS_H
#define _FTDEFS_H

// The maximal number of files and projects to be supported.
#define NUMBER_RECENT_FILES 25
#define NUMBER_RECENT_PROJECTS 25

#define EX_TIMESTAMP_FORMAT "%d %B %Y %H:%M:%S"

// Commands supported by wxfiletool. 
enum
{
  ID_FILETOOL_LOWEST,

  ID_TOOL_SQL = ID_TOOL_FIRST_USER + 1,
  ID_TOOL_REPORT_SQL = ID_TOOL_REPORT_FIRST_USER + 1,
  ID_LIST_LOWEST = ID_TOOL_HIGHEST + 1,
  ID_LIST_ADD_ITEM,
  ID_LIST_TOOL_MENU,
  ID_LIST_COMPARE, 
  ID_LIST_COMPARELAST, 
  ID_LIST_DIFF, 
  ID_LIST_OPEN_ITEM, 
  ID_LIST_RUN_MAKE,
  ID_LIST_VERSIONLIST,
  ID_LIST_SEND_ITEM,
  ID_LIST_HIGHEST,
  ID_LIST_ALL_ITEMS, // not a real command, used by ftForEach
  ID_LIST_ALL_CLOSE, // not a real command, used by ftForEach
  ID_PROJECT_OPEN,
  ID_PROJECT_SAVE, 
  ID_RECENT_LOWEST,
  ID_RECENT_FILE_LOWEST, 
  ID_RECENT_FILE_HIGHEST = ID_RECENT_FILE_LOWEST + NUMBER_RECENT_FILES,
  ID_RECENT_PROJECT_LOWEST,
  ID_RECENT_PROJECT_HIGHEST = ID_RECENT_PROJECT_LOWEST + NUMBER_RECENT_PROJECTS,
  ID_RECENT_HIGHEST,
  ID_SPECIAL_FIND_IN_FILES,
  ID_STC_LOWEST,
  ID_STC_FIND_FILES, 
  ID_STC_REPLACE_FILES,
  ID_STC_COMPARE,
  ID_STC_TOOL_MENU,
  ID_STC_HIGHEST,
  ID_VIEW_STATUSBAR,
  ID_VIEW_TOOLBAR,

  ID_FILETOOL_HIGHEST,
};

#endif
