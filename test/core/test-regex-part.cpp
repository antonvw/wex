////////////////////////////////////////////////////////////////////////////////
// Name:      test-regex-part.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/regex-part.h>
#include <wex/test/test.h>

TEST_CASE("wex::regex_part")
{
  SUBCASE("constructor")
  {
    REQUIRE(
      wex::regex_part(std::string()).match('x') ==
      wex::regex_part::match_t::NONE);
    REQUIRE(wex::regex_part(std::string()).regex() == std::string());
    REQUIRE(wex::regex_part(std::string()).text() == std::string());

    wex::regex_part re("hello", boost::regex::icase);
    REQUIRE(re.match('H') == wex::regex_part::match_t::PART);
    REQUIRE(re.match('E') == wex::regex_part::match_t::PART);
  }

  SUBCASE("match")
  {
    wex::regex_part rer("[xyz");

    REQUIRE(rer.match('x') == wex::regex_part::match_t::ERROR);
    REQUIRE(rer.error().contains("Unmatched"));

    REQUIRE(wex::regex_part("x").match('y') == wex::regex_part::match_t::NONE);
    REQUIRE(wex::regex_part("x").match('x') == wex::regex_part::match_t::FULL);

    wex::regex_part re("\\*+ ");

    REQUIRE(re.match('y') == wex::regex_part::match_t::NONE);
    re.reset();

    REQUIRE(re.match('*') == wex::regex_part::match_t::PART);
    REQUIRE(re.text() == "*");

    REQUIRE(re.match('*') == wex::regex_part::match_t::PART);
    REQUIRE(re.text() == "**");

    REQUIRE(re.match('*') == wex::regex_part::match_t::PART);
    REQUIRE(re.match('*') == wex::regex_part::match_t::PART);
    REQUIRE(re.text() == "****");

    REQUIRE(re.match(' ') == wex::regex_part::match_t::FULL);
    REQUIRE(re.text() == "**** ");

    REQUIRE(re.match('x') == wex::regex_part::match_t::HISTORY);
    REQUIRE(re.text() == "**** ");

    REQUIRE(re.match('y') == wex::regex_part::match_t::HISTORY);
    REQUIRE(re.match_type() == wex::regex_part::match_t::HISTORY);
    REQUIRE(re.text() == "**** ");

    wex::regex_part rfw("\\*+ *Settings? *\\*");
    REQUIRE(rfw.match('*') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('*') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match(' ') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('S') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('e') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('t') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('t') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('i') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('n') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('g') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match(' ') == wex::regex_part::match_t::PART);
    REQUIRE(rfw.match('*') == wex::regex_part::match_t::FULL);
  }

  SUBCASE("reset")
  {
    wex::regex_part re("[abcde]x");

    REQUIRE(re.match('b') == wex::regex_part::match_t::PART);
    REQUIRE(re.match_type() == wex::regex_part::match_t::PART);

    REQUIRE(re.text() == "b");

    re.reset();
    REQUIRE(re.text() == std::string());

    REQUIRE(re.match('x') == wex::regex_part::match_t::NONE);
    REQUIRE(re.match_type() == wex::regex_part::match_t::NONE);
  }
}
