////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/report/defs.h>
#include <wex/report/dirctrl.h>

TEST_CASE("wex::report::dirctrl")
{
  auto* ctrl = new wex::report::dirctrl(report_frame());
  wex::test::add_pane(frame(), ctrl);

  SUBCASE("Select directory") { ctrl->expand_and_select_path("./"); }

#ifdef __UNIX__
  SUBCASE("Select file") { ctrl->expand_and_select_path("./"); }
#endif
}
