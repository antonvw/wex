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

#define STREAM_FIND(RE, FIND, MC, AC)                     \
  frd.set_regex(RE);                                      \
  frd.set_find_string(FIND);                              \
  frd.set_match_case(MC);                                 \
                                                          \
  wex::stream s(                                          \
    &frd,                                                 \
    wex::test::get_path("test.h"),                        \
    wex::tool(wex::ID_TOOL_REPORT_FIND));                 \
                                                          \
  REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND); \
                                                          \
  find_prep(s, &frd);                                     \
                                                          \
  REQUIRE(s.get_statistics().get("Actions Completed") == AC);

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

  // to verify: git grep "\btest\b" test.h | wc

  SUBCASE("find") { STREAM_FIND(false, "test", true, 193); }

  SUBCASE("find-regex") { STREAM_FIND(true, "\\btest\\b", true, 193); }

  SUBCASE("find-ignore-case") { STREAM_FIND(false, "tESt", false, 194); }

  SUBCASE("find-regex-ignore-case")
  {
    STREAM_FIND(true, "\\btESt\\b", false, 194);
  }

  SUBCASE("replace")
  {
    wex::stream s(
      &frd,
      wex::test::get_path("test.h"),
      wex::tool(wex::ID_TOOL_REPLACE));
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPLACE);

    frd.set_regex(false);
    frd.set_find_string("test");
    frd.set_match_case(true);
    find_prep(s, &frd);

    REQUIRE(s.get_statistics().get("Actions Completed") == 196);
  }
}
