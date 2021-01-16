////////////////////////////////////////////////////////////////////////////////
// Name:      test-chrono.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/chrono.h>

TEST_CASE("wex::chrono")
{
  wex::chrono chrono("%Y-%m-%d %H:%M:%S");

  SUBCASE("string")
  {
    REQUIRE(std::get<0>(chrono.get_time("2019-02-01 12:20:06")));
    REQUIRE(!std::get<0>(chrono.get_time("201902-01 12:20:06")));
  }

  SUBCASE("time_t")
  {
    REQUIRE(chrono.get_time(0).find("1970") != std::string::npos);
  }

  SUBCASE("now") { REQUIRE(wex::now().find("202") != std::string::npos); }
}
