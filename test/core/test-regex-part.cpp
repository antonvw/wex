////////////////////////////////////////////////////////////////////////////////
// Name:      test-regex-part.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/core/regex-part.h>

TEST_CASE("wex::regex_part")
{
  SUBCASE("constructor")
  {
    REQUIRE(
      wex::regex_part(std::string()).match('x') == wex::regex_part::MATCH_NONE);
    REQUIRE(wex::regex_part(std::string()).regex() == std::string());
    REQUIRE(wex::regex_part(std::string()).text() == std::string());

    wex::regex_part re("hello", boost::regex::icase);
    REQUIRE(re.match('H') == wex::regex_part::MATCH_PART);
    REQUIRE(re.match('E') == wex::regex_part::MATCH_PART);
  }

  SUBCASE("match")
  {
    wex::regex_part rer("[xyz");

    REQUIRE(rer.match('x') == wex::regex_part::MATCH_ERROR);
    REQUIRE(rer.error().contains("Unmatched"));

    REQUIRE(wex::regex_part("x").match('y') == wex::regex_part::MATCH_NONE);
    REQUIRE(wex::regex_part("x").match('x') == wex::regex_part::MATCH_FULL);

    wex::regex_part re("\\*+ ");

    REQUIRE(re.match('y') == wex::regex_part::MATCH_NONE);
    re.reset();

    REQUIRE(re.match('*') == wex::regex_part::MATCH_PART);
    REQUIRE(re.text() == "*");

    REQUIRE(re.match('*') == wex::regex_part::MATCH_PART);
    REQUIRE(re.text() == "**");

    REQUIRE(re.match('*') == wex::regex_part::MATCH_PART);
    REQUIRE(re.match('*') == wex::regex_part::MATCH_PART);
    REQUIRE(re.text() == "****");

    REQUIRE(re.match(' ') == wex::regex_part::MATCH_FULL);
    REQUIRE(re.text() == "**** ");

    REQUIRE(re.match('x') == wex::regex_part::MATCH_NONE);
    REQUIRE(re.text() == "**** x");

    REQUIRE(re.match('y') == wex::regex_part::MATCH_NONE);
    REQUIRE(re.match_type() == wex::regex_part::MATCH_NONE);
    REQUIRE(re.text() == "**** xy");

    wex::regex_part rfw("\\*+ *Settings? *\\*");
    REQUIRE(rfw.match('*') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('*') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match(' ') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('S') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('e') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('t') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('t') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('i') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('n') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('g') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match(' ') == wex::regex_part::MATCH_PART);
    REQUIRE(rfw.match('*') == wex::regex_part::MATCH_FULL);
  }

  SUBCASE("reset")
  {
    wex::regex_part re("[abcde]x");

    REQUIRE(re.match('b') == wex::regex_part::MATCH_PART);
    REQUIRE(re.match_type() == wex::regex_part::MATCH_PART);

    REQUIRE(re.text() == "b");

    re.reset();
    REQUIRE(re.text() == std::string());

    REQUIRE(re.match('x') == wex::regex_part::MATCH_NONE);
    REQUIRE(re.match_type() == wex::regex_part::MATCH_NONE);
  }
}
