////////////////////////////////////////////////////////////////////////////////
// Name:      test-regex.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/log.h>
#include <wex/core/regex.h>

TEST_CASE("wex::regex")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::regex(std::string()).match("") == 0);
    REQUIRE(wex::regex({"", "", ""}).match("") == 0);
    REQUIRE(wex::regex({"", "", ""}).empty());
    REQUIRE(wex::regex({"", "", ""}).size() == 0);
    REQUIRE(wex::regex("").match_no() == -1);
    REQUIRE(wex::regex("").match_data().text().empty());
  }

  SUBCASE("match")
  {
    REQUIRE(wex::regex("hllo").match("hello world") == -1);
    REQUIRE(wex::regex("hello").match("hello world") == -1);
    REQUIRE(
      wex::regex("([0-9]+)ok([0-9]+)nice").match("xxxx19999ok245nice") == -1);
    REQUIRE(wex::regex("(\\d+)ok(\\d+)nice").match("19999ok245nice") == 2);
    REQUIRE(wex::regex(" ([\\d\\w]+)").match(" 19999ok245nice ") == -1);
    REQUIRE(
      wex::regex("([?/].*[?/])(,[?/].*[?/])([msy])").match("/xx/,/yy/y") == 3);
  }

  SUBCASE("matches")
  {
    wex::regex r(
      {{"(\\.[\\0-7A-Za-z_/.-]+) .*",
        [](const wex::regex::match_t&)
        {
          wex::log::trace("1");
        }},
       {"([0-9]+)([a-z]+)([0-9]+)",
        [](const wex::regex::match_t&)
        {
          wex::log::trace("2");
        }}});

    REQUIRE(r.match("99xx88") == 3);
    REQUIRE(r.size() == 3);
    REQUIRE(r.back() == "88");
    REQUIRE(r.match_data().text() == "([0-9]+)([a-z]+)([0-9]+)");
    REQUIRE(r.match_no() == 1);

    REQUIRE(r.search("aaa .77xx77 xx") == 1);
    REQUIRE(r.match_no() == 0);
    REQUIRE(r[0] == ".77xx77");
  }

  SUBCASE("operator")
  {
    wex::regex r("([?/].*[?/])(,[?/].*[?/])([msy])");

    REQUIRE(r.match("/xx/,/yy/y") == 3);
    REQUIRE(r[0] == "/xx/");
    REQUIRE(r[1] == ",/yy/");
    REQUIRE(r[2] == "y");
  }

  SUBCASE("replace")
  {
    wex::regex r({{"99xx77"}, {"([0-9]+)([a-z]+)([0-9]+)"}});

    std::string text("99xx88");
    REQUIRE(!r.replace(text, "zz"));
    REQUIRE(r.match(text) == 3);
    REQUIRE(r.replace(text, "zz"));
    REQUIRE(text == "zz");
  }

  SUBCASE("search")
  {
    REQUIRE(wex::regex("hllo").search("hello world") == -1);
    REQUIRE(wex::regex("hello").search("hello world") == 0);
    REQUIRE(
      wex::regex("([0-9]+)ok([0-9]+)nice").search("xxxx19999ok245nice") == 2);
    REQUIRE(wex::regex("(\\d+)ok(\\d+)nice").search("19999ok245nice") == 2);
    REQUIRE(wex::regex(" ([\\d\\w]+)").search(" 19999ok245nice ") == 1);
    REQUIRE(
      wex::regex("([?/].*[?/])(,[?/].*[?/])([msy])").match("/xx/,/yy/y") == 3);
  }
}
