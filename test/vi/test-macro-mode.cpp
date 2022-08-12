////////////////////////////////////////////////////////////////////////////////
// Name:      test-macro-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/log-none.h>
#include <wex/vi/macro-mode.h>
#include <wex/vi/macros.h>
#include <wex/vi/variable.h>

TEST_CASE("wex::macro_mode")
{
  wex::macros macros;

  SUBCASE("constructor")
  {
    wex::macro_mode mode(&macros);

    REQUIRE(mode.get_macro().empty());
    REQUIRE(!mode.is_playback());
    REQUIRE(!mode.is_recording());
    REQUIRE(mode.get_macros() == &macros);
  }

  SUBCASE("expand")
  {
    wex::macro_mode mode(&macros);
    std::string     expanded;
    wex::log_none   off;
    REQUIRE(!mode.expand(nullptr, wex::variable("test"), expanded));
  }

  SUBCASE("transition")
  {
    wex::macro_mode mode(&macros);
    REQUIRE(mode.transition("x") == 0);
    REQUIRE(mode.str().empty()); // idle
  }
}
