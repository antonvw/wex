////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/util.h>

#include "test.h"

TEST_CASE("wex::ex::utils")
{
  SUBCASE("append_line_no")
  {
    std::string text;
    wex::append_line_no(text, 1);
    wex::append_line_no(text, 300);
    REQUIRE(text.contains("     2"));
    REQUIRE(text.contains("   301"));
  }

  SUBCASE("esc")
  {
    REQUIRE(wex::esc() == "\x1b");
  }

  SUBCASE("find_first_of")
  {
    REQUIRE(wex::find_first_of("aha", "a") == "ha");
    REQUIRE(wex::find_first_of("aha", "a", 10).empty());
    REQUIRE(wex::find_first_of("aha", "h") == "a");
    REQUIRE(wex::find_first_of("aha", "z").empty());
  }

  SUBCASE("get_lines")
  {
    auto* stc = get_stc();
    stc->set_text("xx\nxx\nyy\nzz\n");

    REQUIRE(wex::get_lines(stc, 0, 0).empty());
    REQUIRE(wex::get_lines(stc, 0, 10000) == "xx\nxx\nyy\nzz\n");
    REQUIRE(
      wex::get_lines(stc, 0, 10000, "ADHJHJHJJKJK#") ==
      "     1 xx\n     2 xx\n     3 yy\n     4 zz\n     5 \n");
    REQUIRE(wex::get_lines(stc, 0, 10000, "l").contains("$"));

    const auto lines(wex::get_lines(stc, 0, 10000, "#l"));
    REQUIRE(lines.contains("$"));
    REQUIRE(lines.contains("    1 xx$\n"));
  }

  SUBCASE("k_s")
  {
    REQUIRE(wex::k_s(WXK_CONTROL_A) == "\x1");
    REQUIRE(wex::k_s(WXK_CONTROL_B) == "\x2");
  }

  SUBCASE("one_letter_after")
  {
    REQUIRE(wex::one_letter_after("m", "mA"));

    REQUIRE(!wex::one_letter_after("", "mA"));
    REQUIRE(!wex::one_letter_after("m", "m"));
    REQUIRE(!wex::one_letter_after("m", "m9"));
  }

  SUBCASE("register_after")
  {
    REQUIRE(wex::register_after("@", "@6"));
    REQUIRE(wex::register_after("@", "@x"));

    REQUIRE(!wex::register_after("@", "@ "));
  }
}
