////////////////////////////////////////////////////////////////////////////////
// Name:      test-path-match.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/path-match.h>
#include <wex/test/test.h>

TEST_CASE("wex::path_match")
{
  wex::path p("xxx");

  SECTION("constructor-path")
  {
    wex::path_match m(p);

    REQUIRE(m.line().empty());
    REQUIRE(m.path() == p);
    REQUIRE(m.line_no() == 0);
    REQUIRE(m.pos() == -1);
    REQUIRE(m.tool().id() == wex::ID_LOWEST);
  }

  SECTION("constructor-all")
  {
    wex::path_match m(p, wex::tool(wex::ID_LIST_MATCH), "hello", 7, 8);

    REQUIRE(m.line() == "hello");
    REQUIRE(m.path() == p);
    REQUIRE(m.line_no() == 7);
    REQUIRE(m.pos() == 8);
    REQUIRE(m.tool().id() == wex::ID_LIST_MATCH);
  }
}
