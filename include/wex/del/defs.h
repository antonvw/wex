////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Common definitions for wex::del
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/defs.h>

namespace wex::del
{
  const int FREE_MAX = 30;

  // Window event ID's supported by wex report.
  enum
  {
    ID_LOWEST = wex::ID_HIGHEST + 1,

    ID_LIST_COMPARE,
    ID_LIST_RUN_MAKE,

    ID_PROJECT_SAVE,

    ID_TREE_COPY,
    ID_TREE_FIND,
    ID_TREE_REPLACE,
    ID_TREE_RUN_MAKE,

    ID_FREE_LOWEST,
    ID_FREE_HIGHEST = ID_FREE_LOWEST + FREE_MAX,

    ID_HIGHEST
  };
}; // namespace wex::del
