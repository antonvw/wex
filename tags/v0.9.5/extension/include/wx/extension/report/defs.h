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

// Commands supported by wxExtension report.
enum
{
  ID_EXTENSION_REPORT_LOWEST = ID_TOOL_FIRST_USER + 1,

  ID_TOOL_SQL,
  ID_TOOL_REPORT_SQL = ID_TOOL_REPORT_FIRST_USER + 1,
  ID_LIST_LOWEST = ID_TOOL_HIGHEST + 1,
  ID_LIST_COMPARE,
  ID_LIST_COMPARELAST,
  ID_LIST_OPEN_ITEM,
  ID_LIST_RUN_MAKE,
  ID_LIST_VERSIONLIST,
  ID_LIST_HIGHEST,
  ID_LIST_SEND_ITEM, // only for wxExListViewFile
  ID_LIST_ALL_ITEMS, // not a real command, used by exForEach
  ID_TREE_OPEN,
  ID_TREE_COPY,
  ID_TREE_FIND,
  ID_TREE_REPLACE,
  ID_TREE_RUN_MAKE,
  ID_LIST_ALL_CLOSE, // not a real command, used by exForEach
  ID_PROJECT_SAVE,
  ID_FIND_IN_FILES,
  ID_REPLACE_IN_FILES,
  ID_TERMINATED_PROCESS,

  ID_EXTENSION_REPORT_HIGHEST,
};

#endif
