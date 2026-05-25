////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../src/stc/util.h"

#include "test.h"

TEST_CASE("wex::util")
{
  SECTION("describe_basefields")
  {
    REQUIRE(wex::describe_basefields("").empty());
    REQUIRE(wex::describe_basefields("zz").empty());

    CAPTURE(wex::describe_basefields("10"));
    REQUIRE(wex::describe_basefields("10").contains("hex as dec: 16"));
    REQUIRE(wex::describe_basefields("10").contains("dec as hex: a"));
    REQUIRE(wex::describe_basefields("10").contains("oct as dec: 8"));

    REQUIRE(wex::describe_basefields("0x10").contains("dec: 16"));
    REQUIRE(!wex::describe_basefields("0x10").contains("oct"));
    REQUIRE(!wex::describe_basefields("0x10").contains("dec as"));

    REQUIRE(wex::describe_basefields("010").contains("oct as dec: 8"));
    REQUIRE(!wex::describe_basefields("010").contains("hex"));
  }
}
