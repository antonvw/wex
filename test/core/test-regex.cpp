////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "../test.h"
#include <wex/regex.h>

TEST_CASE("wex::regex")
{
  SUBCASE("match")
  {
    std::vector<std::string> v;
    REQUIRE(wex::match("hllo", "hello world", v) == -1);
    REQUIRE(wex::match("hello", "hello world", v) == 0);
    REQUIRE(wex::match("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
    REQUIRE(wex::match("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
    REQUIRE(wex::match(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
    REQUIRE(
      wex::match("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  }
}
