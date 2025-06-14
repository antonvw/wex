////////////////////////////////////////////////////////////////////////////////
// Name:      factory/test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/listctrl.h>
#include <wx/menu.h>

#include "test.h"

TEST_CASE("wex::factory::frame")
{
  auto* stc = new wex::test::stc();

  SECTION("bars")
  {
    auto* bar = new wxMenuBar();
    frame()->SetMenuBar(bar);

    frame()->statusbar_clicked("test");

    frame()->statusbar_clicked_right("test");

    frame()->set_recent_file(wex::path("testing"));

    REQUIRE(!frame()->statustext("hello", "test"));
    REQUIRE(!frame()->statustext("hello1", "Pane1"));
    REQUIRE(frame()->get_statustext("Pane1").empty());

    REQUIRE(!frame()->update_statusbar(nullptr, "test"));
    REQUIRE(!frame()->update_statusbar(stc, "test"));

    auto* lv = new wxListView(frame());
    lv->Show();
    REQUIRE(!frame()->update_statusbar(nullptr));
    REQUIRE(!frame()->update_statusbar(lv));
  }

  SECTION("closing")
  {
    REQUIRE(!frame()->is_closing());
  }

  SECTION("coverage")
  {
    frame()->on_command_item_dialog(
      wxID_ADD,
      wxCommandEvent(wxEVT_NULL, wxID_OK));

    REQUIRE(!frame()->output("xxx"));
  }

  SECTION("focus")
  {
    frame()->set_find_focus(stc);
    REQUIRE(frame()->get_find_focus() == stc);
    frame()->set_find_focus(nullptr);
    REQUIRE(frame()->get_find_focus() == nullptr);
    frame()->set_find_focus(frame());
  }

  SECTION("get")
  {
    REQUIRE(frame()->get_grid() == nullptr);
    REQUIRE(frame()->get_listview() == nullptr);
    REQUIRE(frame()->get_process("xxx") == nullptr);
  }

  SECTION("open_file")
  {
    // the factory stc does not open the file
    // the wex::data::stc is not linked, cannot be used
    // will be fixed later
    frame()->set_find_focus(stc);

    const wex::path p("test.h");
    ALLOW_CALL(*stc, path()).RETURN(p);

    REQUIRE(!frame()->is_open(wex::test::get_path("test.h")));
    REQUIRE(!frame()->is_open(wex::path("xxx")));
  }

  SECTION("vcs")
  {
    REQUIRE(
      frame()->vcs_annotate_line(stc, "PaneBlameAuthor") == std::string());
  }
}
