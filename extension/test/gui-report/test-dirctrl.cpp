////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/dir.h> // necessary?
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/defs.h>
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
  wex::dirctrl* ctrl = new wex::dirctrl(GetFrame());
  AddPane(GetFrame(), ctrl);

  SUBCASE("Select directory")
  {
    ctrl->ExpandAndSelectPath("./");
    // Test(ctrl);
  }
  
#ifdef __UNIX__
  SUBCASE("Select file")
  {
    ctrl->ExpandAndSelectPath("/usr/bin/git");
    // Test(ctrl);
  }
#endif
}
