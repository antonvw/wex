////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/listview.h>

#include "test.h"

TEST_CASE("wex::data::listview")
{
  SUBCASE("constructor")
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
    auto* lv = get_listview();
    REQUIRE(wex::data::listview(lv).inject());
    REQUIRE(wex::data::listview(lv, wex::data::control().line(2)).inject());

    REQUIRE(!wex::data::listview().inject());
  }
}
