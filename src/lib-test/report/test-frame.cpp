////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/menu.h>
#include <wex/report/defs.h>
#include <wx/menu.h>

TEST_CASE("wex::report::frame")
{
  auto* list =
    new wex::listview(wex::data::listview().type(wex::data::listview::HISTORY));

  wex::test::add_pane(report_frame(), list);

  auto* menu = new wex::menu();
  report_frame()->use_file_history_list(list);
  report_frame()->get_project_history().use_menu(1000, menu);
  list->Show();

  REQUIRE(!report_frame()->open_file(
    wex::test::get_path("test.h"))); // as we have no focused stc
  REQUIRE(
    report_frame()->file_history().get_history_file().string().find(
      "../test.h") == std::string::npos);

  REQUIRE(!report_frame()->open_file(
    wex::path(get_project()),
    wex::data::stc().flags(wex::data::stc::WIN_IS_PROJECT)));

  wex::find_replace_data::get()->set_find_string("wex::test_app");

  // All find in files, grep fail, because there is no
  // FIND list.

  REQUIRE(!report_frame()->find_in_files({}, wex::ID_TOOL_REPORT_FIND, false));

  REQUIRE(!report_frame()->find_in_files(
    {wex::test::get_path("test.h").string()},
    wex::ID_TOOL_REPORT_FIND,
    false));

  REQUIRE(
    !report_frame()->find_in_files_title(wex::ID_TOOL_REPORT_FIND).empty());

  // It does not open, next should fail.
  REQUIRE(
    report_frame()->get_project_history().get_history_file().string().find(
      get_project()) == std::string::npos);

  REQUIRE(report_frame()->get_project() == nullptr);

  wex::log::verbose(9) << "pwd:" << wex::path::current();

  REQUIRE(!report_frame()->grep("xxxxxxx *.xyz ./"));
  REQUIRE(!report_frame()->grep("xxxxxxx yyy"));
  REQUIRE(!report_frame()->grep("xxxxxxx"));

#ifndef __WXMSW__
  REQUIRE(report_frame()->sed("xxxxxxx yyy *.xyz"));
#endif

  report_frame()->set_recent_project("xxx.prj");
  REQUIRE(report_frame()->get_project_history().get_history_file().empty());

  report_frame()->set_recent_file(wex::test::get_path("test.h"));

  for (auto id : std::vector<int>{wex::ID_CLEAR_PROJECTS,
                                  wex::ID_TOOL_REPORT_FIND,
                                  wex::ID_TOOL_REPLACE,
                                  wex::report::ID_PROJECT_SAVE})
  {
    auto* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(report_frame(), event);
  }
}
