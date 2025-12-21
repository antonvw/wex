////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diff.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/unified-diff.h>

#include "../src/factory/unified-diff-parser.h"
#include "test.h"

TEST_CASE("wex::factory::unified_diff_parser")
{
  SECTION("constructor")
  {
    wex::factory::unified_diff        uni("");
    wex::factory::unified_diff_parser parser(&uni);

    REQUIRE(!parser.parse());
  }

  SECTION("parse-invalid")
  {
    wex::factory::unified_diff uni(
      "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
      "index e4a9c0522..b2637803a 100644\n"
      "--- a/CHANGELOG.md\n"
      "+++ b/CHANGELOG.md\n"
      "@@ -15,0 +16 @@ The format is based on ...\n");

    wex::factory::unified_diff_parser parser(&uni);

    REQUIRE(!parser.parse());
  }

  SECTION("parse")
  {
    wex::factory::unified_diff uni(
      "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
      "index e4a9c0522..b2637803a 100644\n"
      "--- a/CHANGELOG.md\n"
      "+++ b/CHANGELOG.md\n"
      "@@ -15,0 +16 @@ The format is based on ...\n"
      "+- used boost::parser");

    wex::factory::unified_diff_parser parser(&uni);

    REQUIRE(parser.parse());
  }
}
