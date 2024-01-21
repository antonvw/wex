////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wex/core/log-none.h>
#include <wex/core/path.h>
#include <wex/del/defs.h>
#include <wex/stc/link.h>
#include <wex/syntax/blame.h>
#include <wex/syntax/lexers.h>
#include <wex/ui/frd.h>
#include <wex/ui/menu.h>
#include <wex/vcs/process.h>
#include <wex/vcs/vcs.h>

#include "../vcs/test.h"
#include "test.h"

TEST_CASE("wex::del::frame")
{
  SUBCASE("default_extensions")
  {
    REQUIRE(!del_frame()->default_extensions().empty());
  }

  SUBCASE("events")
  {
    for (auto id : std::vector<int>{
           wxID_PREFERENCES,
           wex::ID_CLEAR_FILES,
           wex::ID_CLEAR_PROJECTS,
           wex::ID_FIND_FIRST,
           wex::ID_FIND_LAST,
           wex::del::ID_PROJECT_SAVE,
           // wex::ID_EDIT_VCS_LOWEST,
           wex::ID_VIEW_MENUBAR,
           wex::ID_VIEW_TITLEBAR})
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(del_frame(), event);
      wxTheApp->ProcessPendingEvents();
    }
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

  SUBCASE("vcs_add_path")
  {
    wex::link          lnk;
    wex::data::control data;
    wex::config(_("vcs.Base folder"))
      .set(wex::config::strings_t{wxGetCwd().ToStdString()});
    get_stc()->get_lexer().clear();
    REQUIRE(wex::vcs::load_document());
    REQUIRE(lnk.get_path("modified:  test/vcs/test-vcs.cpp", data, get_stc())
              .file_exists());
  }

  SUBCASE("vcs_annotate_commit")
  {
    wex::config("vcs.VCS").set(2);
    const std::string commit_id("b6aae80e3ab4402c7930a9bd590d355641c74746");

    get_stc()->set_text("line 1\nline 2\nline 3\nline 4\nline 5\n");
    {
      wex::log_none off;
      REQUIRE(!del_frame()->vcs_annotate_commit(get_stc(), 15, commit_id));
      REQUIRE(!del_frame()->vcs_annotate_commit(get_stc(), 4, std::string()));
    }

    REQUIRE(del_frame()->vcs_annotate_commit(get_stc(), 4, commit_id));
  }

  SUBCASE("vcs_blame")
  {
    get_stc()->set_text(std::string());
    {
      wex::config("vcs.VCS").set(-2);
      wex::log_none off;
      REQUIRE(!del_frame()->vcs_blame(get_stc()));
    }
    wex::config("vcs.VCS").set(2);
    REQUIRE(get_stc()->open(wex::test::get_path("test.h")));
    REQUIRE(del_frame()->vcs_blame(get_stc()));

    get_stc()->SetFocus();
    get_stc()->set_margin_text_click(2);
    REQUIRE(get_stc()->get_margin_text_click() == 2);
    REQUIRE(get_stc()->find("b6aae80e3a"));
    get_stc()->SetFocus();
    wxMouseEvent event(wxEVT_LEFT_DOWN);
    wxPostEvent(get_stc(), event);
    wxYield();
    REQUIRE(!get_stc()->find("b6aae80e3a"));
  }

  SUBCASE("vcs_blame_revision")
  {
    wex::config("vcs.VCS").set(2);
    REQUIRE(get_stc()->open(wex::test::get_path("test.h")));
    const std::string renamed;
    const std::string offset;

    REQUIRE(del_frame()->vcs_blame_revision(get_stc(), renamed, offset));
  }

  SUBCASE("vcs_blame_show")
  {
    wex::blame blame;
    wex::lexers::get()->apply_margin_text_style(get_stc(), &blame);
    auto* entry(load_git_entry());

    REQUIRE(!del_frame()->vcs_blame_show(entry, get_stc()));

#ifndef __WXMSW__
    REQUIRE(
      entry->system(wex::process_data().args(
        "blame " + wex::test::get_path("test.h").string())) == 0);
    REQUIRE(del_frame()->vcs_blame_show(entry, get_stc()));
#endif

    get_stc()->get_file().reset_contents_changed();
  }

  SUBCASE("vcs_dir_exists")
  {
    REQUIRE(del_frame()->vcs_dir_exists(wex::test::get_path()));
#ifndef __WXMSW__
    REQUIRE(!del_frame()->vcs_dir_exists(wex::path("/tmp")));
#endif
  }

  SUBCASE("vcs_execute")
  {
    wex::data::window data;
    data.button(wxOK | wxCANCEL | wxAPPLY);
    REQUIRE(del_frame()->vcs_execute(9, {wex::test::get_path("test.h")}, data));
    del_frame()->vcs_destroy_dialog();
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

    del_frame()->on_command_item_dialog(
      wex::del::frame::id_find_in_files,
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
