////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/defs.h>
#include <wex/menu.h>
#include <wex/toolbar.h>

#include "test.h"

TEST_CASE("wex::frame")
{
  get_stc()->SetFocus();

  SUBCASE("open_file")
  {
    // the factory stc does not open the file
    frame()->set_find_focus(get_stc());
    REQUIRE(frame()->open_file(wex::test::get_path("test.h")) != nullptr);
    REQUIRE(!frame()->is_open(wex::test::get_path("test.h")));
    REQUIRE(!frame()->is_open(wex::path("xxx")));
  }

  SUBCASE("get")
  {
    REQUIRE(frame()->get_grid() == nullptr);
    REQUIRE(frame()->get_listview() == nullptr);
    REQUIRE(frame()->get_process("xxx") == nullptr);
    REQUIRE(frame()->get_stc() != nullptr);
  }

  SUBCASE("focus")
  {
    frame()->set_find_focus(frame()->get_stc());
    frame()->set_find_focus(nullptr);
    frame()->set_find_focus(frame());
  }

  SUBCASE("bars")
  {
    auto* bar  = new wxMenuBar();
    auto* menu = new wex::menu();
    menu->append({{wex::menu_item::EDIT}});
    bar->Append(menu, "Edit");
    frame()->SetMenuBar(bar);

    frame()->statusbar_clicked("test");
    frame()->statusbar_clicked("Pane1");
    frame()->statusbar_clicked("Pane2");

    frame()->statusbar_clicked_right("test");
    frame()->statusbar_clicked_right("Pane1");
    frame()->statusbar_clicked_right("Pane2");

    frame()->set_recent_file(wex::path("testing"));

    REQUIRE(!frame()->statustext("hello", "test"));
    REQUIRE(frame()->statustext("hello1", "Pane1"));
    REQUIRE(frame()->statustext("hello2", "Pane2"));
    REQUIRE(frame()->get_statustext("Pane1") == "hello1");
    REQUIRE(frame()->get_statustext("Pane2") == "hello2");

    REQUIRE(!frame()->update_statusbar(frame()->get_stc(), "test"));
    REQUIRE(!frame()->update_statusbar(frame()->get_stc(), "Pane1"));
    REQUIRE(!frame()->update_statusbar(frame()->get_stc(), "Pane2"));
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
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(frame(), event);
      wxQueueEvent(frame(), event);
    }
#endif
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

  REQUIRE(
    frame()->file_history()[0].string().find("test.h") != std::string::npos);
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
