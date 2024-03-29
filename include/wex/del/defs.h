////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Common definitions for wex::del
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/defs.h>

/*! \file */

namespace wex::del
{
const int FREE_MAX = 30;

/// Window event ID's supported by wex del.
enum window_id
{
  ID_LOWEST = wex::ID_HIGHEST + 1,

  ID_LIST_COMPARE,
  ID_LIST_RUN_BUILD,

  ID_PROJECT_SAVE,

  ID_TREE_COPY,
  ID_TREE_FIND,
  ID_TREE_REPLACE,
  ID_TREE_RUN_BUILD,

  ID_FREE_LOWEST,
  ID_FREE_HIGHEST = ID_FREE_LOWEST + FREE_MAX,

  ID_HIGHEST
};
}; // namespace wex::del
