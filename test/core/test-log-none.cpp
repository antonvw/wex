////////////////////////////////////////////////////////////////////////////////
// Name:      test-log-none.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/core/log.h>
#include <wex/test/test.h>

TEST_CASE("wex::log_none")
{
  const auto level(wex::log::get_level());

  {
    wex::log_none off;
    REQUIRE(wex::log::get_level() == wex::log::LEVEL_OFF);
  }

  REQUIRE(wex::log::get_level() == level);
}
