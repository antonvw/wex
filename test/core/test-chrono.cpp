////////////////////////////////////////////////////////////////////////////////
// Name:      test-chrono.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/chrono.h>
#include <wex/test/test.h>

TEST_CASE("wex::chrono")
{
  wex::chrono chrono("%Y-%m-%d %H:%M:%S");

  SECTION("string")
  {
    REQUIRE(chrono.get_time("2019-02-01 12:20:06"));
    REQUIRE(!chrono.get_time("201902-01 12:20:06"));
  }

  SECTION("time_t")
  {
    REQUIRE(chrono.get_time(0).contains("1970"));
  }

  SECTION("precision")
  {
    REQUIRE(chrono.get_time(0).contains("1970"));
    REQUIRE(!chrono.get_time(0).contains("."));
    REQUIRE(wex::chrono("%Y-%m-%d %H:%M:%S", wex::chrono::precision_t::MILLI)
              .get_time(timespec{0, 123000000})
              .contains(".123"));
  }

  SECTION("now")
  {
    REQUIRE(wex::now("%Y-%m-%d %H:%M:%S").contains("202"));
  }
}
