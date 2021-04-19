////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-substitute.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/substitute.h>

#include "../test.h"

TEST_CASE("wex::data::substitute")
{
  SUBCASE("constructor")
  {
    wex::data::substitute sub("x", "y", "z");

    REQUIRE(sub.pattern() == "x");
    REQUIRE(sub.replacement() == "y");
    REQUIRE(!sub.is_confirmed());
    REQUIRE(!sub.is_global());
    REQUIRE(!sub.is_ignore_case());
  }

  SUBCASE("set")
  {
    wex::data::substitute sub("x", "y", "z");
    REQUIRE(!sub.set(""));
    REQUIRE(!sub.set("x"));

    REQUIRE(sub.set("s/xx/yy/c"));
    REQUIRE(sub.pattern() == "xx");
    REQUIRE(sub.replacement() == "yy");
    REQUIRE(sub.is_confirmed());

    REQUIRE(sub.set("s/x/y/g"));
    REQUIRE(!sub.is_ignore_case());
    REQUIRE(sub.is_global());

    REQUIRE(sub.set("s/x/y/i"));
    REQUIRE(sub.is_ignore_case());
    REQUIRE(!sub.is_confirmed());

    REQUIRE(sub.set("s/x/y/cgi"));
    REQUIRE(sub.is_confirmed());
    REQUIRE(sub.is_global());
    REQUIRE(sub.is_ignore_case());
  }
}
