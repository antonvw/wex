////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex.h>

#include "test.h"

TEST_CASE("wex::ex-mode")
{
  auto* stc = get_stc();
  stc->set_text("xx\nxx\nyy\nzz\n");
  auto* ex = new wex::ex(stc, wex::ex::EX);
  stc->DocumentStart();

  SUBCASE("find")
  {
    REQUIRE(ex->visual() == wex::ex::EX);
    REQUIRE(ex->command(":/xx/"));
    REQUIRE(stc->get_current_line() == 1);
    REQUIRE(ex->command("://"));
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(ex->command("://"));
    REQUIRE(stc->get_current_line() == 1);
  }

  ex->use(wex::ex::VISUAL);
}
