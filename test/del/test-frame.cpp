////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/addressrange.h>
#include <wex/del/defs.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/menu.h>
#include <wex/process.h>

#include "test.h"

TEST_CASE("wex::del::frame")
{
  auto* list = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::HISTORY));

  del_frame()->pane_add(list);

  SUBCASE("find_in_files")
  {
    wex::find_replace_data::get()->set_find_string("wex::test_app");

    // All find in files, grep fail, because there is no
    // FIND list.

    REQUIRE(!del_frame()->find_in_files({}, wex::ID_TOOL_REPORT_FIND, false));

#ifndef __WXMSW__
    REQUIRE(!del_frame()->find_in_files(
      {wex::test::get_path("test.h")},
      wex::ID_TOOL_REPORT_FIND,
      false));

    REQUIRE(
      !del_frame()->find_in_files_title(wex::ID_TOOL_REPORT_FIND).empty());

    // It does not open, next should fail.
    REQUIRE(
      del_frame()->get_project_history()[0].string().find(
        get_project().string()) == std::string::npos);

    REQUIRE(del_frame()->get_project() == nullptr);

    REQUIRE(!del_frame()->grep("xxxxxxx *.xyz ./"));
    REQUIRE(!del_frame()->grep("xxxxxxx yyy"));
    REQUIRE(!del_frame()->grep("xxxxxxx"));

    REQUIRE(del_frame()->sed("xxxxxxx yyy *.xyz"));
#endif
  }

  SUBCASE("get_project_history")
  {
    auto* menu = new wex::menu();
    del_frame()->get_project_history().use_menu(1000, menu);
    list->Show();
  }

  SUBCASE("open_file")
  {
    del_frame()->set_find_focus(get_stc());

    REQUIRE(
      ((wex::frame*)del_frame())->open_file(wex::test::get_path("test.h")));
    REQUIRE(
      del_frame()->file_history()[0].string().find("../test.h") ==
      std::string::npos);

    //  REQUIRE(!del_frame()->open_file(
    //    wex::path(get_project()),
    //    wex::data::stc().flags(wex::data::stc::WIN_IS_PROJECT)));
  }

  SUBCASE("process_async_system")
  {
    wex::process::prepare_output(del_frame());
    REQUIRE(wex::addressrange(&get_stc()->get_vi(), "").escape("ls"));
  }

  SUBCASE("set_recent")
  {
    del_frame()->set_recent_project(wex::path("xxx.prj"));
    REQUIRE(del_frame()->get_project_history()[0].empty());

    del_frame()->set_recent_file(wex::test::get_path("test.h"));
  }

  SUBCASE("show_ex_bar")
  {
    del_frame()->show_ex_bar(wex::frame::HIDE_BAR);
    del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FOCUS_STC);
    del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FORCE);
    del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FORCE_FOCUS_STC);
  }

  SUBCASE("stc") { auto* stc = get_stc(); }

  SUBCASE("stc_entry_dialog")
  {
    REQUIRE(del_frame()->stc_entry_dialog_component() != nullptr);
    del_frame()->stc_entry_dialog_title("hello world");
    REQUIRE(del_frame()->stc_entry_dialog_title() == "hello world");
  }

  SUBCASE("use_file_history") { del_frame()->use_file_history_list(list); }

  SUBCASE("visual")
  {
    auto* vi = &get_stc()->get_vi();

    vi->get_stc()->visual(true);
    REQUIRE(!get_stc()->data().flags().test(wex::data::stc::WIN_EX));
    REQUIRE(vi->is_active());

    vi->get_stc()->visual(false);
    CAPTURE(get_stc()->data().flags());
    REQUIRE(get_stc()->data().flags().test(wex::data::stc::WIN_EX));
    REQUIRE(vi->is_active());

    REQUIRE(vi->command(":vi"));
    REQUIRE(!get_stc()->data().flags().test(wex::data::stc::WIN_EX));
    REQUIRE(vi->is_active());
  }

  SUBCASE("other")
  {
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
}
