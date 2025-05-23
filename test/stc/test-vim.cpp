////////////////////////////////////////////////////////////////////////////////
// Name:      test-vim.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/vi.h>

#include "test.h"

TEST_CASE("wex::vim", "[!mayfail]")
{
  auto* stc = get_stc();
  auto* vi  = &get_stc()->get_vi();
  stc->set_text("xxxxxxxxxx second\nxxxxxxxx");

  SECTION("begin")
  {
    REQUIRE(!vi->command("g"));
  }

  SECTION("invalid")
  {
    REQUIRE(vi->command("gc"));
    REQUIRE(vi->command("gcdefg"));
    REQUIRE(vi->command("g5"));
  }

  SECTION("motion")
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

  SECTION("other")
  {
    REQUIRE(vi->command("g8"));
    REQUIRE(vi->command("ga"));
    REQUIRE(vi->command("gd"));
    REQUIRE(vi->command("gf"));
    REQUIRE(vi->command("gm"));

    REQUIRE(vi->command("g*"));
    REQUIRE(vi->command("g#"));

    REQUIRE(vi->command("gt"));
    REQUIRE(vi->command("gT"));
  }

  SECTION("z")
  {
    for (auto& fold : std::vector<
           std::string>{"za", "zo", "zc", "zE", "zf", "zz", "zC", "zO"})
    {
      CAPTURE(fold);
      REQUIRE(vi->command(fold));
      REQUIRE(vi->last_command() != fold);
    }
  }
}
