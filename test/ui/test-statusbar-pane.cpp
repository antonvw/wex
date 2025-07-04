////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar-pane.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/statusbar-pane.h>

#include "test.h"

TEST_CASE("wex::statusbar_pane")
{
  SECTION("PaneInfo")
  {
    wex::statusbar_pane pane("PaneInfo", 15);
    REQUIRE(pane.help_text() == "Lines or Items");
    pane.help("hello");

    REQUIRE(pane.name() == "PaneInfo");
    REQUIRE(pane.help_text() == "hello");
    REQUIRE(pane.hidden_text().empty());
    REQUIRE(pane.GetWidth() == 15);
    REQUIRE(pane.is_shown());
    pane.hidden_text("hidden");
    REQUIRE(pane.hidden_text() == "hidden");
  }

  SECTION("PaneBlameDate")
  {
    wex::statusbar_pane pane("PaneBlameDate", 20);
    REQUIRE(pane.help_text() == "Current line date annotation");
    pane.help("aha");

    REQUIRE(pane.name() == "PaneBlameDate");
    REQUIRE(pane.help_text() == "aha");
    REQUIRE(pane.hidden_text().empty());
    REQUIRE(pane.GetWidth() == 20);
    REQUIRE(pane.is_shown());
    pane.hidden_text("hidden");
    REQUIRE(pane.hidden_text() == "hidden");
  }
}
