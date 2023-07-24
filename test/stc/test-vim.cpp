////////////////////////////////////////////////////////////////////////////////
// Name:      test-vim.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/vi.h>

#include "test.h"

TEST_CASE("wex::vim")
{
  auto* stc = get_stc();
  auto* vi  = &get_stc()->get_vi();
  stc->set_text("xxxxxxxxxx second\nxxxxxxxx");

  SUBCASE("invalid")
  {
    REQUIRE(!vi->command("gc"));
    REQUIRE(!vi->command("gcdefg"));
    REQUIRE(!vi->command("g5"));
  }

  SUBCASE("motion")
  {
    REQUIRE(vi->command("gUw"));
    REQUIRE(vi->get_stc()->get_text() == "XXXXXXXXXX second\nxxxxxxxx");

    REQUIRE(vi->command("gub"));
    REQUIRE(vi->get_stc()->get_text() == "xxxxxxxxxx second\nxxxxxxxx");

    REQUIRE(vi->command("g~w"));
    REQUIRE(vi->get_stc()->get_text() == "XXXXXXXXXX second\nxxxxxxxx");

    REQUIRE(vi->command("b"));
    REQUIRE(vi->command("gU6w"));
    REQUIRE(vi->get_stc()->get_text() == "XXXXXXXXXX SECOND\nXXXXXXXX");
  }

  SUBCASE("special")
  {
    REQUIRE(vi->command("ga"));
    REQUIRE(vi->command("gd"));
    REQUIRE(vi->command("gf"));

    REQUIRE(vi->command("g*"));
    REQUIRE(vi->command("g#"));

    REQUIRE(vi->command("gt"));
  }
}
