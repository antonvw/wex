////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Common definitions for wex report
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/defs.h>

namespace wex::report
{
  // Window event ID's supported by wex report.
  enum
  {
    ID_LOWEST = wex::ID_HIGHEST + 1,

    ID_FIND_IN_FILES,
    ID_REPLACE_IN_FILES,

    ID_LIST_COMPARE,
    ID_LIST_RUN_MAKE,

    ID_PROJECT_SAVE,

    ID_TREE_COPY,
    ID_TREE_FIND,
    ID_TREE_REPLACE,
    ID_TREE_RUN_MAKE,

    ID_HIGHEST
  };
};
