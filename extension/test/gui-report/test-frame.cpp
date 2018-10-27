////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>
#include <wex/frd.h>
#include <wex/report/defs.h>
#include "test.h"
#include <easylogging++.h>

TEST_CASE("wex::history_frame")
{
  wex::listview* list = new wex::listview(wex::listview_data().Type(wex::listview_data::HISTORY));

  AddPane(GetFrame(), list);

  wxMenu* menu = new wxMenu();
  GetFrame()->UseFileHistoryList(list);
  GetFrame()->GetProjectHistory().UseMenu(1000, menu);
  list->Show();
  
  REQUIRE(!GetFrame()->OpenFile(GetTestPath("test.h"))); // as we have no focused stc
  REQUIRE( GetFrame()->GetFileHistory().
    GetHistoryFile().Path().string().find("../test.h") == std::string::npos);

  REQUIRE(!GetFrame()->OpenFile(
    wex::path(GetProject()),
    wex::stc_data().Flags(wex::stc_data::WIN_IS_PROJECT)));
  
  wex::find_replace_data::Get()->SetFindString("wex::test_app");

  // All find in files, grep fail, because there is no
  // FIND list.
  
  REQUIRE(!GetFrame()->FindInFiles({}, wex::ID_TOOL_REPORT_FIND, false));

  REQUIRE(!GetFrame()->FindInFiles(
    {GetTestPath("test.h").Path().string()}, wex::ID_TOOL_REPORT_FIND, false));

  // GetFrame()->FindInFilesDialog(ID_TOOL_REPORT_FIND);
  REQUIRE(!GetFrame()->GetFindInCaption(wex::ID_TOOL_REPORT_FIND).empty());
  
  // It does not open, next should fail.
  REQUIRE( GetFrame()->GetProjectHistory().
    GetHistoryFile().Path().string().find(GetProject()) == std::string::npos);
  
  REQUIRE( GetFrame()->GetProject() == nullptr);

  VLOG(9) << "pwd: " << wex::path::Current();

  REQUIRE(!GetFrame()->Grep("xxxxxxx *.xyz ./"));
  REQUIRE(!GetFrame()->Grep("xxxxxxx yyy"));
  REQUIRE(!GetFrame()->Grep("xxxxxxx"));

#ifndef __WXMSW__
  REQUIRE( GetFrame()->Sed("xxxxxxx yyy *.xyz"));
#endif
  
  GetFrame()->SetRecentProject("xxx.prj");
  REQUIRE( GetFrame()->GetProjectHistory().GetHistoryFile().Path().empty());

  GetFrame()->SetRecentFile(GetTestPath("test.h"));

  for (auto id : std::vector<int> {
    wex::ID_CLEAR_PROJECTS, wex::ID_PROJECT_SAVE, wex::ID_TOOL_REPORT_FIND, wex::ID_TOOL_REPLACE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(GetFrame(), event);
  }
}
