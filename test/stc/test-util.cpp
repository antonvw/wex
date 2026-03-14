////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../src/stc/util.h"

#include "test.h"

TEST_CASE("wex::util")
{
  SECTION("describe_basefields")
  {
    REQUIRE(wex::describe_basefields("").empty());
    REQUIRE(wex::describe_basefields("zz").empty());
    REQUIRE(wex::describe_basefields("10").contains("dec: 16"));
    REQUIRE(wex::describe_basefields("10").contains("hex: a"));
    REQUIRE(wex::describe_basefields("0x10").contains("dec: 16"));
  }
}
