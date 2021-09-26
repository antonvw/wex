////////////////////////////////////////////////////////////////////////////////
// Name:      test-macro-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/vi/macro-mode.h>
#include <wex/vi/macros.h>
#include <wex/vi/variable.h>

TEST_SUITE_BEGIN("wex::vi");

TEST_CASE("wex::macro_mode")
{
  wex::macros     macros;
  wex::macro_mode mode(&macros);

  REQUIRE(!mode.is_playback());
  REQUIRE(!mode.is_recording());
  std::string expanded;
  REQUIRE(!mode.expand(nullptr, wex::variable("test"), expanded));
  REQUIRE(mode.transition("x") == 0);
  REQUIRE(mode.str().empty());
}

TEST_SUITE_END();
