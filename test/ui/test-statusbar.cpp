////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/statusbar.h>

#include "test.h"

TEST_CASE("wex::statusbar")
{
  SUBCASE("setup")
  {
    get_statusbar()->setup(
      frame(),
      {{"Pane0"},
       {"Pane1"},
       {"Pane2"},
       {"Pane3"},
       {"Pane4"},
       {"PaneInfo"},
       {"PaneLexer"},
       {"PaneMode"},
       {"PaneFileType"},
       {"LastPane"}});

    REQUIRE(get_statusbar()->get_pane(0).name() == "PaneText");
    REQUIRE(get_statusbar()->get_pane(1).name() == "Pane0");
    REQUIRE(get_statusbar()->get_pane(1).help_text() == "0");
    REQUIRE(get_statusbar()->GetFieldsCount() >= 10);

    REQUIRE(get_statusbar()->set_statustext("hello", ""));
    REQUIRE(get_statusbar()->set_statustext("hello0", "Pane0"));
    REQUIRE(get_statusbar()->set_statustext("hello1", "Pane1"));
    REQUIRE(get_statusbar()->set_statustext("hello2", "Pane2"));
    REQUIRE(get_statusbar()->set_statustext("hello3", "Pane3"));
    REQUIRE(get_statusbar()->set_statustext("hello4", "Pane4"));
    REQUIRE(!get_statusbar()->set_statustext("helloxxx", "Panexxx"));
    REQUIRE(!get_statusbar()->set_statustext("hello25", "Pane25"));
    REQUIRE(get_statusbar()->set_statustext("GoodBye", "LastPane"));

    REQUIRE(get_statusbar()->get_statustext("Pane0") == "hello0");
    REQUIRE(
      (reinterpret_cast<wxStatusBar*>(get_statusbar()))->GetStatusText(1) ==
      "hello0");
    REQUIRE(get_statusbar()->get_statustext("Pane1") == "hello1");
    REQUIRE(get_statusbar()->get_statustext("Pane2") == "hello2");
    REQUIRE(get_statusbar()->get_statustext("Panexxx").empty());

    REQUIRE(get_statusbar()->pane_show("Pane0", false));
    REQUIRE(
      (reinterpret_cast<wxStatusBar*>(get_statusbar()))->GetStatusText(1) ==
      "hello1");
    REQUIRE(!get_statusbar()->pane_show("Pane0", false));
    REQUIRE(get_statusbar()->pane_show("Pane3", false));
    REQUIRE(get_statusbar()->get_pane(1).name() == "Pane0");
    REQUIRE(
      (reinterpret_cast<wxStatusBar*>(get_statusbar()))->GetStatusText(1) ==
      "hello1");
    REQUIRE(get_statusbar()->get_statustext("Pane0").empty());
    REQUIRE(get_statusbar()->pane_show("Pane0", true));
    REQUIRE(
      (reinterpret_cast<wxStatusBar*>(get_statusbar()))->GetStatusText(1) ==
      "hello0");
    REQUIRE(get_statusbar()->get_statustext("Pane0") == "hello0");
    REQUIRE(get_statusbar()->pane_show("LastPane", false));
    REQUIRE(get_statusbar()->get_statustext("LastPane").empty());
    REQUIRE(!get_statusbar()->set_statustext("BackAgain", "LastPane"));
    REQUIRE(get_statusbar()->pane_show("LastPane", true));
    REQUIRE(get_statusbar()->get_statustext("LastPane") == "BackAgain");
  }

  SUBCASE("statusbar_pane")
  {
    wex::statusbar_pane pane1("PaneInfo", 15);
    REQUIRE(pane1.help_text() == "Lines or Items");
    pane1.help("hello");

    REQUIRE(pane1.name() == "PaneInfo");
    REQUIRE(pane1.help_text() == "hello");
    REQUIRE(pane1.hidden_text().empty());
    REQUIRE(pane1.GetWidth() == 15);
    REQUIRE(pane1.is_shown());
    pane1.hidden_text("hidden");
    REQUIRE(pane1.hidden_text() == "hidden");

    SUBCASE("setup")
    {
      get_statusbar()->setup(
        frame(),
        {pane1, {"PaneLexer"}, {"PaneFileType"}, {"Pane1"}, {"Pane2"}});

      REQUIRE(get_statusbar()->get_pane(0).name() == "PaneText");
      REQUIRE(get_statusbar()->get_pane(0).help_text().empty());
      REQUIRE(get_statusbar()->GetFieldsCount() == 6);
    }

    SUBCASE("setup-with-text")
    {
      get_statusbar()->setup(
        frame(),
        {pane1,
         {"PaneText"},
         {"PaneLexer"},
         {"PaneFileType"},
         {"Pane1"},
         {"Pane2"}});

      REQUIRE(get_statusbar()->get_pane(0).name() == "PaneInfo");
      REQUIRE(get_statusbar()->get_pane(0).help_text() == "hello");
      REQUIRE(get_statusbar()->GetFieldsCount() == 6);
    }
  }
}
