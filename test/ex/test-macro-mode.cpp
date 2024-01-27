////////////////////////////////////////////////////////////////////////////////
// Name:      test-macro-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/ex/ex.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/ex/variable.h>

#include "test.h"

TEST_CASE("wex::macro_mode")
{
  wex::macros macros;

  REQUIRE(macros.load_document());

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
    wex::log_none   off;
    wex::macro_mode mode(&macros);
    REQUIRE(mode.transition("x") == 0);

    REQUIRE(mode.transition("q") == 0);
    REQUIRE(mode.transition("qxxxx") == 5);
    REQUIRE(mode.is_recording());
    REQUIRE(mode.str() == "recording");

    REQUIRE(mode.transition("q") == 1);
    REQUIRE(mode.str().empty()); // idle

    auto* stc = get_stc();
    stc->visual(true);
    auto* ex = new wex::ex(stc);
    REQUIRE(mode.transition("@") == 0);
    REQUIRE(mode.transition("@yyy") == 4);
    REQUIRE(!mode.is_playback());
    REQUIRE(mode.transition("@Hdr@") == 5);
    REQUIRE(!mode.is_playback());
    REQUIRE(mode.transition("@Hdr@", ex) == 5);
    REQUIRE(!mode.is_playback());
    REQUIRE(mode.transition("@Hdr@", ex, false, 6) == 5);
    REQUIRE(!mode.is_playback());
    REQUIRE(mode.str().empty()); // idle
  }
}
