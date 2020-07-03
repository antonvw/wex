////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/listview-data.h>
#include <wex/listview.h>

TEST_CASE("wex::data::listview")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::data::listview().image() == wex::data::listview::IMAGE_ART);
    REQUIRE(wex::data::listview().type() == wex::data::listview::NONE);
    REQUIRE(!wex::data::listview().type_description().empty());
    REQUIRE(
      wex::data::listview().image(wex::data::listview::IMAGE_NONE).image() ==
      wex::data::listview::IMAGE_NONE);
    REQUIRE(
      wex::data::listview(wex::data::control().col(3)).control().col() == 3);
    REQUIRE(
      wex::data::listview(wex::data::window().name("XX")).window().name() ==
      "XX");
  }

  SUBCASE("inject")
  {
    auto* lv = new wex::listview();
    wex::test::add_pane(frame(), lv);
    REQUIRE(wex::data::listview(lv).inject());
    REQUIRE(wex::data::listview(lv, wex::data::control().line(2)).inject());
  }
}
