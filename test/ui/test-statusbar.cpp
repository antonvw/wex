////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/statusbar.h>

#include "test.h"

TEST_CASE("wex::statusbar")
{
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
  REQUIRE(((wxStatusBar*)get_statusbar())->GetStatusText(1) == "hello0");
  REQUIRE(get_statusbar()->get_statustext("Pane1") == "hello1");
  REQUIRE(get_statusbar()->get_statustext("Pane2") == "hello2");
  REQUIRE(get_statusbar()->get_statustext("Panexxx").empty());

  REQUIRE(get_statusbar()->pane_show("Pane0", false));
  REQUIRE(get_statusbar()->get_pane(0).get_name() == "PaneText");
  REQUIRE(((wxStatusBar*)get_statusbar())->GetStatusText(1) == "hello1");
  REQUIRE(!get_statusbar()->pane_show("Pane0", false));
  REQUIRE(get_statusbar()->pane_show("Pane3", false));
  REQUIRE(get_statusbar()->get_pane(1).get_name() == "Pane0");
  REQUIRE(((wxStatusBar*)get_statusbar())->GetStatusText(1) == "hello1");
  REQUIRE(get_statusbar()->get_statustext("Pane0").empty());
  REQUIRE(get_statusbar()->pane_show("Pane0", true));
  REQUIRE(((wxStatusBar*)get_statusbar())->GetStatusText(1) == "hello0");
  REQUIRE(get_statusbar()->get_statustext("Pane0") == "hello0");
  REQUIRE(get_statusbar()->pane_show("LastPane", false));
  REQUIRE(get_statusbar()->get_statustext("LastPane").empty());
  REQUIRE(!get_statusbar()->set_statustext("BackAgain", "LastPane"));
  REQUIRE(get_statusbar()->pane_show("LastPane", true));
  REQUIRE(get_statusbar()->get_statustext("LastPane") == "BackAgain");

  wex::statusbar_pane pane1("PaneInfo", 15);
  pane1.help("hello");

  REQUIRE(pane1.get_name() == "PaneInfo");
  REQUIRE(pane1.help_text() == "hello");
  REQUIRE(pane1.get_hidden_text().empty());
  REQUIRE(pane1.GetWidth() == 15);
  REQUIRE(pane1.is_shown());
  pane1.set_hidden_text("hidden");
  REQUIRE(pane1.get_hidden_text() == "hidden");

  get_statusbar()->setup(
    frame(),
    {pane1, {"PaneLexer"}, {"PaneFileType"}, {"Pane1"}, {"Pane2"}});

  REQUIRE(get_statusbar()->get_pane(0).get_name() == "PaneInfo");
  REQUIRE(get_statusbar()->GetFieldsCount() == 5);
}
