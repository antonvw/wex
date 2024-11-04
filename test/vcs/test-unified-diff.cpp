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
    wex::unified_diff uni("", nullptr);

    REQUIRE(uni.parse());
  }

  SUBCASE("parse-invalid")
  {
    wex::unified_diff uni("error\n", nullptr);

    REQUIRE(!uni.parse());
  }

  SUBCASE("parse-valid")
  {
    wex::unified_diff uni(
      "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
      "index a23525b3c..26e9e8fc1 100644\n"
      "--- a/CHANGELOG.md\n"
      "+++ b/CHANGELOG.md\n"
      "@@ -10,0 +11 @@ The format is based on [Keep a Changelog].\n"
      "+- added git diff option\n",
      nullptr);

    REQUIRE(uni.parse());
  }
}
