////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/report/dir.h> // necessary?
#include <wex/report/dirctrl.h>
#include <wex/report/defs.h>
#include "test.h"

void Test(wex::dirctrl* ctrl)
{
  for (auto id : std::vector<int> {
    wex::ID_EDIT_VCS_LOWEST + 2, 
    wex::ID_TREE_COPY, 
    wex::ID_EDIT_OPEN, 
    wex::ID_TREE_RUN_MAKE,
    wex::ID_TOOL_REPORT_FIND})
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(ctrl, event);
  }
}

TEST_CASE("wex::dirctrl")
{
  wex::dirctrl* ctrl = new wex::dirctrl(frame());
  add_pane(frame(), ctrl);

  SUBCASE("Select directory")
  {
    ctrl->expand_and_select_path("./");
    // Test(ctrl);
  }
  
#ifdef __UNIX__
  SUBCASE("Select file")
  {
    ctrl->expand_and_select_path("./");
    // Test(ctrl);
  }
#endif
}
