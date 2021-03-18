////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/del/defs.h>
#include <wex/del/dirctrl.h>

TEST_CASE("wex::del::dirctrl")
{
  auto* ctrl = new wex::del::dirctrl(del_frame());
  add_pane(del_frame(), ctrl);

  SUBCASE("Select directory") { ctrl->expand_and_select_path("./"); }

#ifdef __UNIX__
  SUBCASE("Select file") { ctrl->expand_and_select_path("./"); }
#endif
}
