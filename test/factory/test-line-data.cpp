////////////////////////////////////////////////////////////////////////////////
// Name:      test-line-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/line-data.h>
#include <wex/test/test.h>

TEST_CASE("wex::line_data")
{
  SECTION("constructor")
  {
    wex::line_data data;

    REQUIRE(wex::line_data().col() == wex::NUMBER_NOT_SET);
    REQUIRE(wex::line_data().command().empty());
    REQUIRE(!wex::line_data().is_ctag());
    REQUIRE(wex::line_data().line() == wex::NUMBER_NOT_SET);
  }

  SECTION("reset")
  {
    wex::line_data data(wex::line_data().line(3));
    data.reset();
    REQUIRE(data.line() == wex::NUMBER_NOT_SET);
  }

  SECTION("set")
  {
    REQUIRE(wex::line_data().col(3).col() == 3);
    REQUIRE(wex::line_data().command("xx").command() == "xx");
    REQUIRE(wex::line_data().is_ctag(true).is_ctag());
    REQUIRE(wex::line_data().line(-1).line() == -1);
    REQUIRE(wex::line_data().line(3).line() == 3);
  }
}
