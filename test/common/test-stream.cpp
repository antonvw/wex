////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/stream.h>
#include <wex/factory/frd.h>

#include <wex/test/test.h>

#include <chrono>

void find_prep(wex::stream& s, wex::factory::find_replace_data* frd)
{
  REQUIRE(s.path() == wex::test::get_path("test.h"));

  frd->set_replace_string("test");

  const auto start = std::chrono::system_clock::now();
  REQUIRE(s.run_tool());
  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - start);
  REQUIRE(milli.count() < 1000);

  REQUIRE(!s.get_statistics().get_elements().get_items().empty());
}

#define STREAM_FIND(FIND, IS_RE, IS_MC, IS_MW, AC)                             \
  frd.set_find_string(FIND);                                                   \
  frd.set_match_case(IS_MC);                                                   \
  frd.set_match_word(IS_MW);                                                   \
  frd.set_regex(IS_RE);                                                        \
                                                                               \
  wex::stream s(                                                               \
    &frd,                                                                      \
    wex::test::get_path("test.h"),                                             \
    wex::tool(wex::ID_TOOL_REPORT_FIND));                                      \
                                                                               \
  REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);                      \
                                                                               \
  find_prep(s, &frd);                                                          \
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
  REQUIRE(frd.data() != nullptr);

  // to verify:
  // git grep -i "test" test.h | wc
  // git grep "\btest\b" test.h | wc

  // no regex

  SECTION("find-is-mc-is-mw")
  {
    STREAM_FIND("TEST", false, true, true, 2);
  }

  SECTION("find-is-mc-no-mw")
  {
    STREAM_FIND("TEST", false, true, false, 3);
  }

  SECTION("find-no-mc-is-mw")
  {
    STREAM_FIND("TEST", false, false, true, 193);
  }

  SECTION("find-no-mc-no-mw")
  {
    STREAM_FIND("TEST", false, false, false, 197);
  }

  // regex

  SECTION("find-regex-is-mc-is-mw")
  {
    STREAM_FIND("\\btest\\b", true, true, true, 190);
  }

  SECTION("find-regex-is-mc-no-mw")
  {
    STREAM_FIND("TEST", true, true, false, 3);
  }

  SECTION("find-regex-no-mc-is-mw")
  {
    STREAM_FIND("\\btESt\\b", true, false, true, 193);
  }

  SECTION("find-regex-no-mc-no-mw")
  {
    STREAM_FIND("TEST", true, false, false, 197);
  }

  SECTION("replace")
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

    REQUIRE(s.get_statistics().get("Actions Completed") == 195);
  }
}
