////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/regex.h>

TEST_CASE("wex::regex")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::regex(std::string()).match("") == 0);
    REQUIRE(wex::regex({"", "", ""}).match("") == 0);
    REQUIRE(wex::regex({"", "", ""}).matches().empty());
    REQUIRE(wex::regex({"", "", ""}).size() == 0);
  }

  SUBCASE("operator")
  {
    wex::regex r("([?/].*[?/])(,[?/].*[?/])([msy])");

    REQUIRE(r.match("/xx/,/yy/y") == 3);
    REQUIRE(r[0] == "/xx/");
    REQUIRE(r[1] == ",/yy/");
    REQUIRE(r[2] == "y");
  }

  SUBCASE("match")
  {
    REQUIRE(wex::regex("hllo").match("hello world") == -1);
    REQUIRE(wex::regex("hello").match("hello world") == 0);
    REQUIRE(wex::regex("([0-9]+)ok([0-9]+)nice").match("19999ok245nice") == 2);
    REQUIRE(wex::regex("(\\d+)ok(\\d+)nice").match("19999ok245nice") == 2);
    REQUIRE(wex::regex(" ([\\d\\w]+)").match(" 19999ok245nice ") == 1);
    REQUIRE(
      wex::regex("([?/].*[?/])(,[?/].*[?/])([msy])").match("/xx/,/yy/y") == 3);
  }

  SUBCASE("matches")
  {
    wex::regex r({std::string("99xx77"), "([0-9]+)([a-z]+)([0-9]+)"});

    REQUIRE(r.match("99xx88") == 3);
    REQUIRE(r.which().second == "([0-9]+)([a-z]+)([0-9]+)");
    REQUIRE(r.which_no() == 1);
  }
}
