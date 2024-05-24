////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/listctrl.h>

#include <wex/factory/defs.h>
#include <wex/factory/process-data.h>
#include <wex/ui/menu.h>
#include <wex/ui/toolbar.h>

#include "test.h"

void browse_check(
  bool                  backward,
  bool                  forward,
  bool                  browse,
  const wxCommandEvent& ev)
{
  REQUIRE(frame()->allow_browse_backward() == backward);
  REQUIRE(frame()->allow_browse_forward() == forward);
  REQUIRE(frame()->browse(ev) == browse);
}

TEST_CASE("wex::frame")
{
  get_stc()->SetFocus();

  SUBCASE("bars")
  {
    auto* bar  = new wxMenuBar();
    auto* menu = new wex::menu();
    menu->append({{wex::menu_item::EDIT}});
    bar->Append(menu, "Edit");
    frame()->SetMenuBar(bar);

    frame()->setup_statusbar(std::vector<wex::statusbar_pane>{
      {{"Pane0"}, {"Pane1"}, {"Pane2"}, {"Pane3"}, {"Pane4"}, {"PaneInfo"}}});

    frame()->statusbar_clicked_right("test");
    frame()->statusbar_clicked_right("Pane1");
    frame()->statusbar_clicked_right("Pane2");

    frame()->set_recent_file(wex::path("testing"));

    REQUIRE(!frame()->statustext("hello", "test"));
    REQUIRE(frame()->statustext("hello1", "Pane1"));
    REQUIRE(frame()->statustext("hello2", "Pane2"));
    REQUIRE(frame()->get_statustext("Pane1") == "hello1");
    REQUIRE(frame()->get_statustext("Pane2") == "hello2");

    REQUIRE(!frame()->update_statusbar(get_stc(), "PaneX"));
    REQUIRE(!frame()->update_statusbar(get_stc(), "Pane1"));
    REQUIRE(frame()->update_statusbar(get_stc(), "PaneInfo"));

    auto* lv = new wxListView(frame());
    lv->Show();
    REQUIRE(frame()->update_statusbar(lv));
  }

  SUBCASE("browse")
  {
    frame()->file_history().clear();

    wxCommandEvent forward(wxEVT_MENU, wxID_FORWARD);
    wxCommandEvent backward(wxEVT_MENU, wxID_BACKWARD);

    browse_check(false, false, false, forward);
    browse_check(false, false, false, backward);

    frame()->set_recent_file(wex::path(wex::test::get_path("test.h")));
    frame()->set_recent_file(wex::path(wex::test::get_path("test.md")));
    frame()->set_recent_file(wex::path(wex::test::get_path("test.bin")));
    REQUIRE(frame()->file_history().size() == 3);

    // bools:: allow backward, allow forward, browse
    browse_check(false, true, true, forward);
    browse_check(true, true, true, forward);
    browse_check(true, false, false, forward);
    browse_check(true, false, true, backward);
    browse_check(true, true, true, backward);
    browse_check(false, true, false, backward);
  }

  SUBCASE("coverage")
  {
    auto*                           menu = new wex::menu();
    wex::menu_item                  i;
    std::vector<wxAcceleratorEntry> ve;

    wex::ex_command           command;
    const wex::frame::panes_t panes;

    frame()->append_vcs(menu, &i);

    frame()->bind_accelerators(frame(), ve, false);

    frame()->debug_add_menu(*menu, true);

    frame()->debug_exe(1000, get_stc());

    frame()->debug_exe("hello", get_stc());

    frame()->debug_exe(wex::test::get_path("test.h"));

    REQUIRE(frame()->debug_handler() == nullptr);

    REQUIRE(!frame()->debug_is_active());

    REQUIRE(!frame()->debug_print("xxx"));

    REQUIRE(!frame()->debug_toggle_breakpoint(1000, get_stc()));

    REQUIRE(!frame()->exec_ex_command(command));

    REQUIRE(!frame()->is_address(get_stc(), "pppp"));

    frame()->on_notebook(100, nullptr);

    frame()->open_file_same_page(wex::test::get_path("test.h"));

    REQUIRE(!frame()->print_ex(get_stc(), "hello"));

    REQUIRE(!frame()->process_async_system(wex::process_data()));

    frame()->record("aha");

    REQUIRE(frame()->restore_page("xyz") == nullptr);

    REQUIRE(!frame()->save_current_page("abc"));

    frame()->shift_double_click();

    frame()->show_ex_bar(wex::frame::HIDE_BAR_FOCUS_STC, nullptr);

    frame()->show_ex_message("this is a message");

    frame()->show_stc_entry_dialog();

    REQUIRE(frame()->stc_entry_dialog_component() == nullptr);

    REQUIRE(frame()->stc_entry_dialog_title().empty());

    frame()->stc_entry_dialog_title("cccc");

    frame()->stc_entry_dialog_validator("a.*b");

    frame()->sync_all();

    frame()->sync_close_all(100);

    frame()->vcs_add_path(nullptr);

    frame()->vcs_annotate_commit(get_stc(), 100, "a898989aaabbb");

    frame()->vcs_blame(get_stc());

    frame()->vcs_blame_revision(get_stc(), "renamed", "100");

    REQUIRE(!frame()->vcs_dir_exists(wex::test::get_path()));

    frame()->vcs_execute(55, std::vector<wex::path>{wex::test::get_path()});

    auto* stc = new wex::test::ui_stc();
    REQUIRE(!frame()->pane_add(stc).empty());

    REQUIRE(frame()->pane_add(panes));

    REQUIRE(frame()->pane_get("vvvvvvvv") == nullptr);

    REQUIRE(!frame()->pane_is_maximized("vvvvvvvv"));

    REQUIRE(frame()->pane_is_shown("vvvvvvvv"));

    REQUIRE(!frame()->pane_maximize("vvvvvvvv"));

    REQUIRE(!frame()->pane_restore("vvvvvvvv"));

    const wxAuiPaneInfo info;
    REQUIRE(!frame()->pane_set("vvvvvvvv", info));

    REQUIRE(!frame()->pane_show("vvvvvvvv", true));

    REQUIRE(!frame()->pane_toggle("vvvvvvvv"));

    REQUIRE(frame()->panes() > 0);

    frame()->set_debug_entry(nullptr);

    REQUIRE(
      frame()->setup_statusbar(std::vector<wex::statusbar_pane>{{}}) !=
      nullptr);

    REQUIRE(!frame()->show_ex_command(get_stc(), "label"));

    REQUIRE(frame()->show_ex_input(get_stc(), 'c'));

    frame()->show_process(false);

    REQUIRE(!frame()->toggled_panes().empty());
  }

  SUBCASE("events")
  {
    wxCommandEvent event(wxEVT_MENU, wxID_OPEN);

    for (const auto& str :
         std::vector<std::string>{"xxx", "+10 test", "`pwd`", "0"})
    {
      event.SetString(str);
      wxPostEvent(frame(), event);
    }

#ifndef __WXMSW__
    for (const auto& id : std::vector<int>{
           wxID_FIND,
           wxID_REPLACE,
           wex::ID_VIEW_MENUBAR,
           wex::ID_VIEW_STATUSBAR,
           wex::ID_VIEW_TITLEBAR})
    {
      wxCommandEvent event(wxEVT_MENU, id);
      wxPostEvent(frame(), event);
    }
#endif
  }

  SUBCASE("focus")
  {
    frame()->set_find_focus(frame()->get_stc());
    frame()->set_find_focus(nullptr);
    frame()->set_find_focus(frame());
  }

  SUBCASE("get")
  {
    REQUIRE(frame()->get_grid() == nullptr);
    REQUIRE(frame()->get_listview() == nullptr);
    REQUIRE(frame()->get_process("xxx") == nullptr);
  }

  SUBCASE("open_file")
  {
    // the factory stc does not open the file
    frame()->set_find_focus(get_stc());
    REQUIRE(frame()->open_file(wex::test::get_path("test.h")) != nullptr);
    REQUIRE(!frame()->is_open(wex::test::get_path("test.h")));
    REQUIRE(!frame()->is_open(wex::path("xxx")));
  }
}

// Also test the toolbar (wex::toolbar).
TEST_CASE("wex::frame::bars")
{
  REQUIRE(frame()->allow_close(100, nullptr));

  get_stc()->SetFocus();
  get_stc()->Show();

  wex::ex_command command(":n");
  REQUIRE(!frame()->exec_ex_command(command));

  REQUIRE(!frame()->show_ex_command(get_stc(), ""));
  REQUIRE(!frame()->show_ex_command(get_stc(), "x"));
  REQUIRE(!frame()->show_ex_command(get_stc(), "xx"));
  REQUIRE(frame()->show_ex_command(get_stc(), "/"));
  REQUIRE(frame()->show_ex_command(get_stc(), "?"));
  REQUIRE(
    frame()->show_ex_command(get_stc(), std::string(1, WXK_CONTROL_R) + "="));

  REQUIRE(!frame()->save_current_page("key"));
  REQUIRE(frame()->restore_page("key") == nullptr);

  frame()->file_history().clear();

  auto* menu = new wex::menu();
  frame()->file_history().use_menu(1000, menu);
  frame()->set_find_focus(frame()->get_stc());
  frame()->open_file(wex::test::get_path("test.h"));

  frame()->set_recent_file(wex::path(wex::test::get_path("test.h")));
  frame()->set_recent_file(wex::path("testing"));

  REQUIRE(frame()->file_history()[0].string().contains("test.h"));
  REQUIRE(frame()->file_history().size() > 0);
  REQUIRE(!frame()->file_history().get_history_files(5).empty());

  frame()->show_ex_message("hello from frame()");
  REQUIRE(!frame()->pane_show("xxxx"));
  REQUIRE(!frame()->pane_show("xxxx", false));

  frame()->sync_all();
  frame()->sync_close_all(0);

  REQUIRE(frame()->get_find_toolbar() != nullptr);
  REQUIRE(frame()->get_options_toolbar() != nullptr);
  REQUIRE(frame()->get_toolbar() != nullptr);

  frame()->get_toolbar()->add_standard();
  REQUIRE(frame()->pane_toggle("FINDBAR"));
  REQUIRE(frame()->pane_is_shown("FINDBAR"));
  REQUIRE(frame()->pane_toggle("OPTIONSBAR"));
  REQUIRE(frame()->pane_is_shown("OPTIONSBAR"));
  REQUIRE(frame()->pane_toggle("TOOLBAR"));
  REQUIRE(!frame()->pane_is_shown("TOOLBAR"));
  REQUIRE(frame()->pane_show("TOOLBAR"));
  REQUIRE(!frame()->pane_toggle("XXXXBAR"));

  frame()->on_notebook(100, get_stc());

#ifndef __WXMSW__
  for (auto id : std::vector<int>{
         wxID_PREFERENCES,
         wex::ID_FIND_FIRST,
         wex::ID_FIND_LAST,
         wex::ID_CLEAR_FILES,
         wex::ID_CLEAR_FINDS,
         wex::ID_VIEW_LOWEST + 1,
         wex::ID_VIEW_LOWEST + 2,
         wex::ID_VIEW_LOWEST + 3,
         wex::ID_VIEW_LOWEST + 4})
  {
    auto* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(frame(), event);
  }
#endif
}
