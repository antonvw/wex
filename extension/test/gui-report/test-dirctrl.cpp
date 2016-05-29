////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/dir.h> // necessary?
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/defs.h>
#include "test.h"

void Test(wxExGenericDirCtrl* ctrl)
{
  for (auto id : std::vector<int> {
    ID_EDIT_VCS_LOWEST + 2, 
    ID_TREE_COPY, 
    ID_EDIT_OPEN, 
    ID_TREE_RUN_MAKE,
    ID_TOOL_REPORT_FIND})
  {
    wxPostEvent(ctrl, wxCommandEvent(wxEVT_MENU, id));
    REQUIRE(wxExUIAction(ctrl, "key"));
  }
}

TEST_CASE("wxExDirCtrl")
{
  wxExGenericDirCtrl* ctrl = new wxExGenericDirCtrl(GetFrame(), GetFrame());
  AddPane(GetFrame(), ctrl);

  SECTION("Select directory")
  {
    ctrl->ExpandAndSelectPath("./");
    Test(ctrl);
  }
  
#ifdef __UNIX__
  SECTION("Select file")
  {
    ctrl->ExpandAndSelectPath("/usr/bin/git");
    Test(ctrl);
  }
#endif
}
