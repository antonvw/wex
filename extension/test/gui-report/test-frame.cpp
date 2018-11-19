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
  wex::listview* list = new wex::listview(wex::listview_data().type(wex::listview_data::HISTORY));

  AddPane(frame(), list);

  wxMenu* menu = new wxMenu();
  frame()->use_file_history_list(list);
  frame()->get_project_history().use_menu(1000, menu);
  list->Show();
  
  REQUIRE(!frame()->open_file(GetTestPath("test.h"))); // as we have no focused stc
  REQUIRE( frame()->file_history().
    get_history_file().data().string().find("../test.h") == std::string::npos);

  REQUIRE(!frame()->open_file(
    wex::path(get_project()),
    wex::stc_data().flags(wex::stc_data::WIN_IS_PROJECT)));
  
  wex::find_replace_data::get()->set_find_string("wex::test_app");

  // All find in files, grep fail, because there is no
  // FIND list.
  
  REQUIRE(!frame()->find_in_files({}, wex::ID_TOOL_REPORT_FIND, false));

  REQUIRE(!frame()->find_in_files(
    {GetTestPath("test.h").data().string()}, wex::ID_TOOL_REPORT_FIND, false));

  // frame()->find_in_files_dialog(ID_TOOL_REPORT_FIND);
  REQUIRE(!frame()->find_in_files_title(wex::ID_TOOL_REPORT_FIND).empty());
  
  // It does not open, next should fail.
  REQUIRE( frame()->get_project_history().
    get_history_file().data().string().find(get_project()) == std::string::npos);
  
  REQUIRE( frame()->get_project() == nullptr);

  VLOG(9) << "pwd: " << wex::path::current();

  REQUIRE(!frame()->grep("xxxxxxx *.xyz ./"));
  REQUIRE(!frame()->grep("xxxxxxx yyy"));
  REQUIRE(!frame()->grep("xxxxxxx"));

#ifndef __WXMSW__
  REQUIRE( frame()->sed("xxxxxxx yyy *.xyz"));
#endif
  
  frame()->set_recent_project("xxx.prj");
  REQUIRE( frame()->get_project_history().get_history_file().data().empty());

  frame()->set_recent_file(GetTestPath("test.h"));

  for (auto id : std::vector<int> {
    wex::ID_CLEAR_PROJECTS, wex::ID_PROJECT_SAVE, wex::ID_TOOL_REPORT_FIND, wex::ID_TOOL_REPLACE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(frame(), event);
  }
}
