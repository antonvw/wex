////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diffs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vcs/unified-diff.h>
#include <wex/vcs/unified-diffs.h>

#include "test.h"

TEST_CASE("wex::unified_diffs")
{
  SUBCASE("constructor")
  {
    wex::unified_diffs diffs(get_stc());
    
    REQUIRE(diffs.size() == 0);
    REQUIRE(diffs.distance() == 0);
    REQUIRE(!diffs.first());
    REQUIRE(!diffs.next());
    REQUIRE(!diffs.prev());
  }
    
  SUBCASE("insert")
  {
    wex::unified_diff uni(
      "diff --git a/build-gen.sh b/build-gen.sh\n"
      "index 9ff921d..b429c21 100755\n"
      "--- a/build-gen.sh\n"
      "+++ b/build-gen.sh\n"
      "@@ -20 +19,0 @@ usage()\n"
      "-  echo \"-D <x=y> add a general cmake define\"\n"
      "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
      "index 26e9e8f..ed2116e 100644\n"
      "--- a/CHANGELOG.md\n"
      "+++ b/CHANGELOG.md\n"
      "@@ -11 +10,0 @@ The format is based on [Keep a Changelog].\n"
      "-- added git diff option\n"
      "@@ -25 +23,0 @@ The format is based on [Keep a Changelog].\n"
      "-- listview standard column sizes are configurable\n"
      "@@ -38,0 +37 @@ The format is based on [Keep a Changelog].\n"
      "+- test\n");

    REQUIRE(uni.parse());

    wex::unified_diffs diffs(get_stc());
    diffs.insert(&uni);
    
    REQUIRE(diffs.size() == 2); // only last one
    REQUIRE(diffs.distance() == 0);
    REQUIRE(diffs.next());
    REQUIRE(diffs.distance() == 1);
    REQUIRE(!diffs.next()); // we are on the last element
    REQUIRE(diffs.prev());
    REQUIRE(!diffs.prev());

    diffs.insert(&uni); // same result
    REQUIRE(diffs.size() == 2);
    
    diffs.clear();
    REQUIRE(diffs.size() == 0);
    REQUIRE(!diffs.next());
    REQUIRE(!diffs.prev());
  }
}
