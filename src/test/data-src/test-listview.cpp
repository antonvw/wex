////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/listview-data.h>
#include <wex/listview.h>

TEST_CASE("wex::listview_data")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::listview_data().image() == wex::listview_data::IMAGE_ART);
    REQUIRE(wex::listview_data().type() == wex::listview_data::NONE);
    REQUIRE(!wex::listview_data().type_description().empty());
    REQUIRE(
      wex::listview_data().image(wex::listview_data::IMAGE_NONE).image() ==
      wex::listview_data::IMAGE_NONE);
    REQUIRE(
      wex::listview_data(wex::control_data().col(3)).control().col() == 3);
    REQUIRE(
      wex::listview_data(wex::window_data().name("XX")).window().name() ==
      "XX");
  }

  SUBCASE("inject")
  {
    auto* lv = new wex::listview();
    wex::test::add_pane(frame(), lv);
    REQUIRE(wex::listview_data(lv).inject());
    REQUIRE(wex::listview_data(lv, wex::control_data().line(2)).inject());
  }
}
