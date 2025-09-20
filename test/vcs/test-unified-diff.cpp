////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diff.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
    REQUIRE(uni.path_vcs().empty());
  }
}
