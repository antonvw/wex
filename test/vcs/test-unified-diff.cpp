////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diff.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/process/v1/system.hpp>
#include <wex/core/log-none.h>
#include <wex/vcs/unified-diff.h>

#include "test.h"

TEST_CASE("wex::unified_diff")
{
  SECTION("constructor")
  {
    const wex::path   p("test.h");
    auto*             entry = load_git_entry();
    wex::unified_diff uni(p, entry, frame());

    wex::log_none off; // currently no diffs
    REQUIRE(uni.parse());
    REQUIRE(uni.range_from_count() == 0);
    REQUIRE(uni.range_to_count() == 0);
    CAPTURE(uni.toplevel_path().string());
    REQUIRE(uni.toplevel_path().string().contains("wex"));
    REQUIRE(!uni.report_path().empty());
    REQUIRE(uni.differences() == 0);
  }

  SECTION("diff")
  {
    const wex::path p("test.h");
    boost::process::v1::system("gsed -i -e 1,6d " + p.string());

    auto* entry = load_git_entry();
    entry->system(
      wex::process_data().args(
        "diff -U0 " + wex::test::get_path("test.h").string()));

    wex::unified_diff uni(p, entry, frame());

    REQUIRE(uni.parse());
    REQUIRE(uni.range_from_count() == 6);
    REQUIRE(uni.range_to_count() == 0);
    REQUIRE(uni.report_path().string().contains("test.h"));
    REQUIRE(uni.differences() == 1);

    entry->system(
      wex::process_data().args(
        "checkout " + wex::test::get_path("test.h").string()));
  }
}
