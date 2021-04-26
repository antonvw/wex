////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/factory/frd.h>
#include <wex/stream.h>

#include "../test.h"

void find_prep(wex::stream& s, wex::factory::find_replace_data* frd)
{
  REQUIRE(s.get_filename() == wex::test::get_path("test.h"));

  frd->set_match_word(true);
  frd->set_replace_string("test");

  const auto start = std::chrono::system_clock::now();
  REQUIRE(s.run_tool());
  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - start);

  REQUIRE(!s.get_statistics().get_elements().get_items().empty());
}

TEST_CASE("wex::stream_statistics")
{
  wex::stream_statistics ss;

  REQUIRE(ss.get().empty());
  REQUIRE(ss.get("xx") == 0);

  wex::stream_statistics ss2;
  REQUIRE(ss2.get().empty());

  ss += ss2;

  REQUIRE(ss.get().empty());
}

TEST_CASE("wex::stream")
{
  wex::factory::find_replace_data frd;

  SUBCASE("find")
  {
    wex::stream s(
      &frd,
      wex::test::get_path("test.h"),
      wex::ID_TOOL_REPORT_FIND);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);

    frd.set_regex(false);
    frd.set_find_string("test");
    frd.set_match_case(true);

    find_prep(s, &frd);

    // to verify: git grep "\btest\b" test.h | wc
    REQUIRE(s.get_statistics().get("Actions Completed") == 193);
  }

  SUBCASE("find-regex")
  {
    wex::stream s(
      &frd,
      wex::test::get_path("test.h"),
      wex::ID_TOOL_REPORT_FIND);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);

    frd.set_regex(true);
    frd.set_find_string("\\btest\\b");
    frd.set_match_case(true);

    find_prep(s, &frd);

    // to verify: git grep "\btest\b" test.h | wc
    REQUIRE(s.get_statistics().get("Actions Completed") == 193);
  }

  SUBCASE("find-ignore-case")
  {
    wex::stream s(
      &frd,
      wex::test::get_path("test.h"),
      wex::ID_TOOL_REPORT_FIND);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);

    frd.set_regex(false);
    frd.set_find_string("tESt");
    frd.set_match_case(false);
    find_prep(s, &frd);

    // to verify: git grep -i "\btest\b" test.h | wc
    REQUIRE(s.get_statistics().get("Actions Completed") == 194);
  }

  SUBCASE("find-regex-ignore-case")
  {
    wex::stream s(
      &frd,
      wex::test::get_path("test.h"),
      wex::ID_TOOL_REPORT_FIND);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);

    frd.set_regex(true);
    frd.set_find_string("\\btESt\\b");
    frd.set_match_case(false);
    find_prep(s, &frd);

    // to verify: git grep -i "\btest\b" test.h | wc
    REQUIRE(s.get_statistics().get("Actions Completed") == 194);
  }

  SUBCASE("replace")
  {
    wex::stream s(&frd, wex::test::get_path("test.h"), wex::ID_TOOL_REPLACE);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPLACE);

    frd.set_regex(false);
    frd.set_find_string("test");
    frd.set_match_case(true);
    find_prep(s, &frd);

    REQUIRE(s.get_statistics().get("Actions Completed") == 196);
  }
}
