////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs-entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/vcs/unified-diff.h>

#include "test.h"

TEST_CASE("wex::unified_diff")
{
  SUBCASE("constructor")
  {
    wex::unified_diff uni("");

    REQUIRE(uni.parse());
    REQUIRE(uni.range_from_count() == 0);
    REQUIRE(uni.range_to_count() == 0);
  }

  SUBCASE("parse-invalid")
  {
    wex::log_none off;
    REQUIRE(!wex::unified_diff("error\n").parse());
    REQUIRE(!wex::unified_diff(
               "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
               "index a23525b3c..26e9e8fc1 100644\n"
               "--- a/CHANGELOG.md\n"
               "+++ b/CHANGELOG.md\n"
               "@@ -10,0 + @@ The format is based on [Keep a Changelog].\n"
               "+- added git diff option\n")
               .parse());
    REQUIRE(!wex::unified_diff("diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
                               "index a23525b3c..26e9e8fc1 100644\n"
                               "--- a/CHANGELOG.md\n"
                               "+++ b/CHANGELOG.md\n"
                               "+- added git diff option\n")
               .parse());
  }

  SUBCASE("parse-valid")
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

    const auto res(uni.parse());
    REQUIRE(res);
    REQUIRE(*res == 4);
    REQUIRE(uni.path_from().string() == "CHANGELOG.md");
    REQUIRE(uni.path_to().string() == "CHANGELOG.md");
    REQUIRE(uni.range_from_start() == 38);
    REQUIRE(uni.range_from_count() == 0);
    REQUIRE(uni.range_to_start() == 37);
    REQUIRE(uni.range_to_count() == 1);
    REQUIRE(uni.text_added().front() == "- test");
    REQUIRE(uni.text_removed().empty());
  }
}
