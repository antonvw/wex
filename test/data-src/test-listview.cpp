////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/listview.h>

#include "test.h"

TEST_CASE("wex::data::listview")
{
  SUBCASE("constructor")
  {
    wex::data::listview data;

    REQUIRE(data.image() == wex::data::listview::IMAGE_ART);
    REQUIRE(data.type() == wex::data::listview::NONE);
    REQUIRE(data.get_listview() == nullptr);
    REQUIRE(!data.type_description().empty());
    REQUIRE(!data.revision());
    REQUIRE(data.menu().all());
    REQUIRE(data.lexer() == nullptr);
    REQUIRE(
      data.image(wex::data::listview::IMAGE_NONE).image() ==
      wex::data::listview::IMAGE_NONE);
    REQUIRE(
      wex::data::listview(wex::data::control().col(3)).control().col() == 3);
    REQUIRE(
      wex::data::listview(wex::data::window().name("XX")).window().name() ==
      "XX");
  }

  SUBCASE("inject")
  {
    wex::data::listview data;
    data.set_listview(get_listview());

    REQUIRE(data.inject());
    REQUIRE(data.get_listview() == get_listview());
    REQUIRE(!data.control(wex::data::control().line(2)).inject());

    REQUIRE(!wex::data::listview().inject());
  }

  SUBCASE("set")
  {
    wex::data::listview data;

    REQUIRE(!data.revision());
    data.revision(true);
    REQUIRE(data.revision());
  }
}
