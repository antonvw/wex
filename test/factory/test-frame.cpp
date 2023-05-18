////////////////////////////////////////////////////////////////////////////////
// Name:      factory/test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>

#include <wex/factory/frame.h>

#include "test.h"

TEST_CASE("wex::factory::frame")
{
  auto* stc = new wex::test::stc();

  SUBCASE("bars")
  {
    auto* bar = new wxMenuBar();
    frame()->SetMenuBar(bar);

    frame()->statusbar_clicked("test");
    frame()->statusbar_clicked("Pane1");
    frame()->statusbar_clicked("Pane2");

    frame()->statusbar_clicked_right("test");
    frame()->statusbar_clicked_right("Pane1");
    frame()->statusbar_clicked_right("Pane2");

    frame()->set_recent_file(wex::path("testing"));

    REQUIRE(!frame()->statustext("hello", "test"));
    REQUIRE(!frame()->statustext("hello1", "Pane1"));
    REQUIRE(frame()->get_statustext("Pane1").empty());
    REQUIRE(frame()->get_statustext("Pane2").empty());

    REQUIRE(!frame()->update_statusbar(stc, "test"));
    REQUIRE(!frame()->update_statusbar(stc, "Pane1"));
    REQUIRE(!frame()->update_statusbar(stc, "Pane2"));
  }

  SUBCASE("closing")
  {
    REQUIRE(!frame()->is_closing());
  }

  SUBCASE("coverage")
  {
    frame()->on_command_item_dialog(
      wxID_ADD,
      wxCommandEvent(wxEVT_NULL, wxID_OK));

    REQUIRE(!frame()->output("xxx"));
  }

  SUBCASE("focus")
  {
    frame()->set_find_focus(stc);
    REQUIRE(frame()->get_find_focus() == stc);
    frame()->set_find_focus(nullptr);
    REQUIRE(frame()->get_find_focus() == nullptr);
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
    // the wex::data::stc is not linked, cannot be used
    // will be fixed later
    frame()->set_find_focus(stc);
    REQUIRE(!frame()->is_open(wex::test::get_path("test.h")));
    REQUIRE(!frame()->is_open(wex::path("xxx")));
  }
}
