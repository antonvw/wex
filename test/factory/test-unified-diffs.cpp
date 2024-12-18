////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diffs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/unified-diffs.h>

#include "test.h"

TEST_CASE("wex::unified_diffs")
{
  auto* stc = new wex::test::stc();

  for (int l = 0; l < 50; l++)
  {
    stc->append_text("this is a line\n");
  }

  SUBCASE("constructor")
  {
    wex::unified_diffs diffs(stc);

    REQUIRE(diffs.size() == 0);
    REQUIRE(diffs.pos() == 0);
    REQUIRE(!diffs.first());
    REQUIRE(!diffs.next());
    REQUIRE(!diffs.prev());
    REQUIRE(!diffs.checkout(0));
  }

  SUBCASE("insert")
  {
    wex::factory::unified_diff uni(
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
      "@@ -38,1 +37 @@ The format is based on [Keep a Changelog].\n"
      "-- added git diff option\n"
      "+- test\n");

    REQUIRE(uni.parse());

    wex::unified_diffs diffs(stc);
    diffs.insert(&uni);

    REQUIRE(diffs.size() == 2); // only last difference
    REQUIRE(diffs.pos() == 1);
    REQUIRE(diffs.next());
    REQUIRE(stc->get_current_line() == 36);
    REQUIRE(diffs.pos() == 1);
    REQUIRE(diffs.next()); // we are on the last element
    REQUIRE(diffs.pos() == 2);
    REQUIRE(diffs.prev());
    REQUIRE(!diffs.prev());
    REQUIRE(diffs.end());
    REQUIRE(diffs.pos() == 2);

    // do a checkout
    REQUIRE(diffs.prev());
    REQUIRE(diffs.pos() == 1);
    REQUIRE(stc->get_current_line() == 36);
    REQUIRE(diffs.checkout(36));
    REQUIRE(diffs.size() == 1);
    CAPTURE(stc->get_text());
    REQUIRE(stc->get_text().contains("added git diff option"));

    diffs.insert(&uni); // back to first
    REQUIRE(diffs.pos() == 1);
    REQUIRE(diffs.size() == 2);

    diffs.clear();
    REQUIRE(diffs.size() == 0);
    REQUIRE(!diffs.next());
    REQUIRE(!diffs.prev());
    REQUIRE(diffs.pos() == 0);
  }

  SUBCASE("insert-other")
  {
    wex::factory::unified_diff uni(
      "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
      "index 26e9e8f..ed2116e 100644\n"
      "--- a/CHANGELOG.md\n"
      "+++ b/CHANGELOG.md\n"
      "@@ -38,0 +37 @@ The format is based on [Keep a Changelog].\n"
      "+- test\n");

    REQUIRE(uni.parse());

    wex::unified_diffs diffs(stc);
    diffs.insert(&uni);

    REQUIRE(diffs.size() == 1);
    REQUIRE(diffs.pos() == 1);
    REQUIRE(diffs.next());
    REQUIRE(!diffs.next());
    REQUIRE(stc->get_current_line() == 36);
  }
}
