////////////////////////////////////////////////////////////////////////////////
// Name:      defs.h
// Purpose:   Constant definitions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/report/defs.h>

// Command id's used.
enum
{
  ID_APPL_LOWEST = wex::ID_REPORT_HIGHEST + 1,
  ID_EDIT_MACRO,
  ID_EDIT_MACRO_MENU,
  ID_EDIT_MACRO_PLAYBACK,
  ID_EDIT_MACRO_START_RECORD,
  ID_EDIT_MACRO_STOP_RECORD,
  ID_NOTEBOOK_EDITORS,
  ID_NOTEBOOK_LISTS,
  ID_NOTEBOOK_PROJECTS,
  ID_OPTION_LIST,
  ID_OPTION_TAB,
  ID_SPLIT,
  ID_SPLIT_HORIZONTALLY,
  ID_SPLIT_VERTICALLY,
  ID_VIEW_DIRCTRL,
  ID_VIEW_HISTORY,
  ID_APPL_HIGHEST
};
