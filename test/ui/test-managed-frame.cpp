////////////////////////////////////////////////////////////////////////////////
// Name:      test-managed_frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/defs.h>
#include <wex/managed-frame.h>
#include <wex/menu.h>
#include <wex/stc.h>
#include <wex/toolbar.h>
#include <wex/vi.h>

// Also test the toolbar (wex::toolbar).
TEST_CASE("wex::managed_frame")
{
  REQUIRE(frame()->allow_close(100, nullptr));

  get_stc()->SetFocus();
  get_stc()->Show();
  auto* vi = &get_stc()->get_vi();

  wex::ex_command command(":n");
  REQUIRE(!frame()->exec_ex_command(command));

  REQUIRE(!frame()->show_ex_command(vi, ""));
  REQUIRE(!frame()->show_ex_command(vi, "x"));
  REQUIRE(!frame()->show_ex_command(vi, "xx"));
  REQUIRE(frame()->show_ex_command(vi, "/"));
  REQUIRE(frame()->show_ex_command(vi, "?"));
  REQUIRE(frame()->show_ex_command(vi, std::string(1, WXK_CONTROL_R) + "="));

  REQUIRE(!frame()->save_current_page("key"));
  REQUIRE(frame()->restore_page("key") == nullptr);

  frame()->show_ex_bar(wex::managed_frame::HIDE_BAR);
  frame()->show_ex_bar(wex::managed_frame::HIDE_BAR_FOCUS_STC);
  frame()->show_ex_bar(wex::managed_frame::HIDE_BAR_FORCE);
  frame()->show_ex_bar(wex::managed_frame::HIDE_BAR_FORCE_FOCUS_STC);

  REQUIRE(!frame()->pane_is_shown("VIBAR"));

  frame()->file_history().clear();

  auto* menu = new wex::menu();
  frame()->file_history().use_menu(1000, menu);
  frame()->set_find_focus(frame()->get_stc());
  frame()->open_file(wex::test::get_path("test.h"));

  frame()->set_recent_file(wex::test::get_path("test.h"));
  frame()->set_recent_file("testing");

  REQUIRE(
    frame()->file_history().get_history_file().string().find("test.h") !=
    std::string::npos);
  REQUIRE(frame()->file_history().size() > 0);
  REQUIRE(!frame()->file_history().get_history_files(5).empty());

  frame()->show_ex_message("hello from frame()");
  REQUIRE(!frame()->pane_show("xxxx"));
  REQUIRE(!frame()->pane_show("xxxx", false));
  frame()->print_ex(vi, "hello vi");

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
  REQUIRE(frame()->pane_toggle("VIBAR"));
  REQUIRE(frame()->pane_is_shown("VIBAR"));
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
