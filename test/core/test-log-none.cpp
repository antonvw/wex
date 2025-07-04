////////////////////////////////////////////////////////////////////////////////
// Name:      test-log-none.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/core/log.h>
#include <wex/test/test.h>

TEST_CASE("wex::log_none")
{
  const auto level(wex::log::get_level());

  REQUIRE(level != wex::log::level_t::OFF);

  {
    wex::log_none off;
    REQUIRE(wex::log::get_level() == wex::log::level_t::OFF);
  }

  REQUIRE(wex::log::get_level() == level);

  wex::log_none off;
  REQUIRE(wex::log::get_level() == wex::log::level_t::OFF);

  off.enable();
  REQUIRE(wex::log::get_level() == level);
}
