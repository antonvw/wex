////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Common definitions for wex report
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/defs.h>

namespace wex
{
  // Commands supported by wex report.
  enum
  {
    ID_REPORT_LOWEST = ID_TOOL_HIGHEST + 1,

    ID_LIST_COMPARE,
    ID_LIST_RUN_MAKE,
    ID_LIST_HIGHEST,
    ID_TREE_COPY,
    ID_TREE_FIND,
    ID_TREE_REPLACE,
    ID_TREE_RUN_MAKE,
    ID_PROJECT_SAVE,
    ID_FIND_IN_FILES,
    ID_REPLACE_IN_FILES,

    ID_REPORT_HIGHEST
  };
};
