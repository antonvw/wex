////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/defs.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/menu.h>

#include "test.h"

TEST_CASE("wex::del::frame")
{
  auto* list =
    new wex::listview(wex::data::listview().type(wex::data::listview::HISTORY));

  del_frame()->pane_add(list);

  auto* menu = new wex::menu();
  del_frame()->use_file_history_list(list);
  del_frame()->get_project_history().use_menu(1000, menu);
  list->Show();

  del_frame()->set_find_focus(get_stc());

  REQUIRE(((wex::frame*)del_frame())->open_file(wex::test::get_path("test.h")));
  REQUIRE(
    del_frame()->file_history().get_history_file().string().find("../test.h") ==
    std::string::npos);

  //  REQUIRE(!del_frame()->open_file(
  //    wex::path(get_project()),
  //    wex::data::stc().flags(wex::data::stc::WIN_IS_PROJECT)));

  wex::find_replace_data::get()->set_find_string("wex::test_app");

  // All find in files, grep fail, because there is no
  // FIND list.

  REQUIRE(!del_frame()->find_in_files({}, wex::ID_TOOL_REPORT_FIND, false));

  REQUIRE(!del_frame()->find_in_files(
    {wex::test::get_path("test.h").string()},
    wex::ID_TOOL_REPORT_FIND,
    false));

  REQUIRE(!del_frame()->find_in_files_title(wex::ID_TOOL_REPORT_FIND).empty());

  // It does not open, next should fail.
  REQUIRE(
    del_frame()->get_project_history().get_history_file().string().find(
      get_project()) == std::string::npos);

  REQUIRE(del_frame()->get_project() == nullptr);

  wex::log::trace("pwd") << wex::path::current();

  REQUIRE(!del_frame()->grep("xxxxxxx *.xyz ./"));
  REQUIRE(!del_frame()->grep("xxxxxxx yyy"));
  REQUIRE(!del_frame()->grep("xxxxxxx"));

#ifndef __WXMSW__
  REQUIRE(del_frame()->sed("xxxxxxx yyy *.xyz"));
#endif

  del_frame()->set_recent_project("xxx.prj");
  REQUIRE(del_frame()->get_project_history().get_history_file().empty());

  del_frame()->set_recent_file(wex::test::get_path("test.h"));

  del_frame()->show_ex_bar(wex::frame::HIDE_BAR);
  del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FOCUS_STC);
  del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FORCE);
  del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FORCE_FOCUS_STC);

  auto* vi = &get_stc()->get_vi();
  del_frame()->print_ex(vi, "hello vi");

  REQUIRE(!del_frame()->pane_is_shown("VIBAR"));

  for (auto id : std::vector<int>{
         wex::ID_CLEAR_PROJECTS,
         wex::ID_TOOL_REPORT_FIND,
         wex::ID_TOOL_REPLACE,
         wex::del::ID_PROJECT_SAVE})
  {
    auto* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(del_frame(), event);
  }
}
