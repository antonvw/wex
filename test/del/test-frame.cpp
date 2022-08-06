////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wex/core/log.h>
#include <wex/del/defs.h>
#include <wex/stc/process.h>
#include <wex/ui/frd.h>
#include <wex/ui/menu.h>

#include "test.h"

TEST_CASE("wex::del::frame")
{
  SUBCASE("default_extensions")
  {
    REQUIRE(!del_frame()->default_extensions().empty());
  }

  SUBCASE("find_in_files")
  {
    wex::find_replace_data::get()->set_find_string("wex::test_app");

    REQUIRE(!del_frame()
               ->find_in_files({}, wex::tool(wex::ID_TOOL_REPORT_FIND), false));

#ifndef __WXMSW__
    REQUIRE(del_frame()->find_in_files(
      {wex::test::get_path("test.h")},
      wex::tool(wex::ID_TOOL_REPORT_FIND),
      false));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE(
      !del_frame()->find_in_files_title(wex::ID_TOOL_REPORT_FIND).empty());

    // It does not open, next should fail.
    REQUIRE(
      del_frame()->get_project_history()[0].string().find(
        get_project().string()) == std::string::npos);

    REQUIRE(del_frame()->get_project() == nullptr);

    REQUIRE(del_frame()->grep("xxxxxxx *.xyz ./"));
    REQUIRE(del_frame()->grep("xxxxxxx yyy"));
    REQUIRE(del_frame()->grep("xxxxxxx"));

    REQUIRE(del_frame()->sed("xxxxxxx yyy *.xyz"));
#endif
  }

  SUBCASE("get_debug")
  {
    REQUIRE(del_frame()->get_debug() != nullptr);
  }

  SUBCASE("get_project_history")
  {
    auto* menu = new wex::menu();
    del_frame()->get_project_history().use_menu(1000, menu);
  }

  SUBCASE("open_file")
  {
    del_frame()->set_find_focus(get_stc());

    REQUIRE(
      ((wex::frame*)del_frame())->open_file(wex::test::get_path("test.h")));
    REQUIRE(
      del_frame()->file_history()[0].string().find("../test.h") ==
      std::string::npos);
  }

  SUBCASE("prepare_output")
  {
    wex::process::prepare_output(del_frame());
    REQUIRE(get_stc()->get_vi().command("!ls"));
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

  SUBCASE("statustext_vcs")
  {
    del_frame()->statustext_vcs(get_stc());
  }

  SUBCASE("stc_entry_dialog")
  {
    REQUIRE(del_frame()->stc_entry_dialog_component() != nullptr);
    del_frame()->stc_entry_dialog_title("hello world");
    REQUIRE(del_frame()->stc_entry_dialog_title() == "hello world");
  }

  SUBCASE("sync")
  {
    del_frame()->sync(false);
    del_frame()->sync(true);
  }

  SUBCASE("use_file_history")
  {
    auto* list = new wex::del::listview(
      wex::data::listview().type(wex::data::listview::HISTORY));
    del_frame()->pane_add(list);
    list->Show();
    del_frame()->use_file_history_list(list);
    REQUIRE(del_frame()->activate(wex::data::listview::HISTORY) != nullptr);
  }

  SUBCASE("virtual")
  {
    auto*          menu = new wex::menu();
    wex::menu_item item;

    wex::log_none off;

    del_frame()->append_vcs(menu, &item);

    const std::vector<wxAcceleratorEntry> v{};
    del_frame()->bind_accelerators(del_frame(), v);

    del_frame()->debug_add_menu(*menu, true);

    del_frame()->debug_exe(100, get_stc());

    del_frame()->debug_exe("gdb", get_stc());

    REQUIRE(del_frame()->debug_handler() != nullptr);

    REQUIRE(!del_frame()->debug_is_active());

    REQUIRE(!del_frame()->debug_print("hello"));

    REQUIRE(!del_frame()->debug_toggle_breakpoint(100, get_stc()));

    REQUIRE(!del_frame()->is_address(get_stc(), "xx"));
    REQUIRE(del_frame()->is_address(get_stc(), "1,5ya"));
    REQUIRE(del_frame()->is_address(get_stc(), "%ya"));

    del_frame()->on_command_item_dialog(
      wxID_ADD,
      wxCommandEvent(wxEVT_NULL, wxID_OK));

    del_frame()->on_notebook(100, nullptr);

#ifndef __WXMSW__
    del_frame()->process_async_system(wex::process_data("ls"));
#endif

    del_frame()->set_recent_file(wex::path("file"));

    del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FOCUS_STC);
    del_frame()->show_ex_bar(wex::frame::HIDE_BAR_FOCUS_STC, get_stc());

    del_frame()->show_ex_message("hello");

    del_frame()->statusbar_clicked("text");

    del_frame()->statusbar_clicked_right("text");

    REQUIRE(del_frame()->show_stc_entry_dialog());

    REQUIRE(del_frame()->stc_entry_dialog_component() != nullptr);

    del_frame()->stc_entry_dialog_title("hello world");

    REQUIRE(del_frame()->stc_entry_dialog_title() == "hello world");

    del_frame()->stc_entry_dialog_validator("choose [0-9]");
  }

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
