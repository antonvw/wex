////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/dirctrl.h>

#include "test.h"

TEST_CASE("wex::del::dirctrl")
{
  auto* ctrl = new wex::del::dirctrl(del_frame());
  del_frame()->pane_add(ctrl);

  SECTION("directory")
  {
    ctrl->expand_and_select_path(wex::path("./"));
  }

#ifdef __UNIX__
  SECTION("file")
  {
    ctrl->expand_and_select_path(wex::path("./"));
  }
#endif
}
