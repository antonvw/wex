////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <chrono>
#include <wex/frd.h>
#include <wex/stream.h>

void find_prep(wex::stream& s)
{
  REQUIRE(s.get_filename() == wex::test::get_path("test.h"));

  wex::find_replace_data::get()->set_find_string("test");
  wex::find_replace_data::get()->set_match_case(true);
  wex::find_replace_data::get()->set_match_word(true);
  wex::find_replace_data::get()->set_regex(false);
  wex::find_replace_data::get()->set_replace_string("test");

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
  SUBCASE("find")
  {
    wex::stream s(wex::test::get_path("test.h"), wex::ID_TOOL_REPORT_FIND);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);

    find_prep(s);

    REQUIRE(s.get_statistics().get("Actions Completed") == 193);
  }

  SUBCASE("replace")
  {
    wex::stream s(wex::test::get_path("test.h"), wex::ID_TOOL_REPLACE);
    REQUIRE(s.get_tool().id() == wex::ID_TOOL_REPLACE);

    find_prep(s);

    REQUIRE(s.get_statistics().get("Actions Completed") == 196);
  }
}
