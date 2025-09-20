////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/util.h>

#include "../../src/ex/util.h"
#include "test.h"

TEST_CASE("wex::ex::utils")
{
  SECTION("append_line_no")
  {
    std::string text;
    wex::append_line_no(text, 1);
    wex::append_line_no(text, 300);
    REQUIRE(text.contains("     2"));
    REQUIRE(text.contains("   301"));
  }

  SECTION("esc")
  {
    REQUIRE(wex::esc() == "\x1b");
  }

  SECTION("find_from")
  {
    const std::vector<std::pair<std::string, std::string>> none{};
    const std::vector<std::pair<std::string, std::string>> cmds{
      {"x", "1"},
      {"abc", "1"},
      {"DDDD", "3"}};

    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        none,
        "") == none.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "") == cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "x") != cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "xx") != cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "ab") != cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "abcd") != cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "d") == cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "DDD") != cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "DDD",
        true) == cmds.end());
    REQUIRE(
      wex::find_from<std::vector<std::pair<std::string, std::string>>>(
        cmds,
        "DDDD",
        true) != cmds.end());
  }

  SECTION("find_first_of")
  {
    REQUIRE(wex::find_first_of("aha", "a") == "ha");
    REQUIRE(wex::find_first_of("aha", "a", 10).empty());
    REQUIRE(wex::find_first_of("aha", "h") == "a");
    REQUIRE(wex::find_first_of("aha", "z").empty());
  }

  SECTION("get_lines")
  {
    auto* stc = new wex::test::stc();
    stc->set_text("xx\nxx\nyy\nzz\n");

    const wex::path p("test.h");
    ALLOW_CALL(*stc, path()).RETURN(p);

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

  SECTION("is_register_valid")
  {
    REQUIRE(wex::is_register_valid("@6"));
    REQUIRE(wex::is_register_valid("@x"));

    REQUIRE(!wex::is_register_valid("@6 "));
    REQUIRE(!wex::is_register_valid("@ "));
    REQUIRE(!wex::is_register_valid("x"));
    REQUIRE(!wex::is_register_valid("xx"));
  }

  SECTION("k_s")
  {
    REQUIRE(wex::k_s(WXK_CONTROL_A) == "\x1");
    REQUIRE(wex::k_s(WXK_CONTROL_B) == "\x2");
  }

  SECTION("to_reverse")
  {
    REQUIRE(wex::to_reverse("") == "");
    REQUIRE(wex::to_reverse("1") == "1");
    REQUIRE(wex::to_reverse("aBcDe") == "AbCdE");
  }
}
