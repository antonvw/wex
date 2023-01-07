////////////////////////////////////////////////////////////////////////////////
// Name:      test-regex-part.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/regex-part.h>

TEST_CASE("wex::regex_part")
{
  SUBCASE("constructor")
  {
    REQUIRE(
      wex::regex_part(std::string()).match('x') == wex::regex_part::MATCH_NONE);
    REQUIRE(wex::regex_part(std::string()).regex() == std::string());
    REQUIRE(wex::regex_part(std::string()).text() == std::string());

    wex::regex_part re("hello", std::regex::icase);
    REQUIRE(re.match('H') == wex::regex_part::MATCH_ALL);
    REQUIRE(re.match('E') == wex::regex_part::MATCH_ALL);
  }

  SUBCASE("match")
  {
    wex::regex_part rer("[xyz");

    REQUIRE(rer.match('x') == wex::regex_part::MATCH_ERROR);
    REQUIRE(rer.error().contains("mismatched"));

    REQUIRE(wex::regex_part("x").match('y') == wex::regex_part::MATCH_NONE);
    REQUIRE(wex::regex_part("x").match('x') == wex::regex_part::MATCH_COMPLETE);

    wex::regex_part re("\\*+ ");

    REQUIRE(re.match('y') == wex::regex_part::MATCH_NONE);
    re.reset();

    REQUIRE(re.match('*') == wex::regex_part::MATCH_ALL);
    REQUIRE(re.text() == "*");
    REQUIRE(re.part() == "\\*+");

    REQUIRE(re.match('*') == wex::regex_part::MATCH_ALL);
    REQUIRE(re.text() == "**");
    REQUIRE(re.part() == "\\*+");

    REQUIRE(re.match('*') == wex::regex_part::MATCH_ALL);
    REQUIRE(re.match('*') == wex::regex_part::MATCH_ALL);
    REQUIRE(re.text() == "****");
    REQUIRE(re.part() == "\\*+");

    REQUIRE(re.match(' ') == wex::regex_part::MATCH_COMPLETE);
    REQUIRE(re.text() == "**** ");
    REQUIRE(re.part() == "\\*+ ");

    REQUIRE(re.match('x') == wex::regex_part::MATCH_PART);
    REQUIRE(re.text() == "**** x");
    REQUIRE(re.part() == "\\*+ ");

    REQUIRE(re.match('y') == wex::regex_part::MATCH_PART);
    REQUIRE(re.match_type() == wex::regex_part::MATCH_PART);
    REQUIRE(re.text() == "**** x");
    REQUIRE(re.part() == "\\*+ ");

    wex::regex_part rfw("\\*+ *Settings? *\\*");
    REQUIRE(rfw.match('*') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('*') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match(' ') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('S') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('e') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('t') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('t') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('i') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('n') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('g') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match(' ') == wex::regex_part::MATCH_ALL);
    REQUIRE(rfw.match('*') == wex::regex_part::MATCH_COMPLETE);
  }

  SUBCASE("reset")
  {
    wex::regex_part re("[abcde]x");

    REQUIRE(re.match('b') == wex::regex_part::MATCH_ALL);
    REQUIRE(re.match_type() == wex::regex_part::MATCH_ALL);

    REQUIRE(re.text() == "b");
    REQUIRE(re.part() == "[abcde]");

    re.reset();
    REQUIRE(re.text() == std::string());
    REQUIRE(re.part() == std::string());

    REQUIRE(re.match('x') == wex::regex_part::MATCH_NONE);
    REQUIRE(re.match_type() == wex::regex_part::MATCH_NONE);
  }
}
