////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diff.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025-2026 Anton van Wezenbeek
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

    REQUIRE(parser.parse());
    REQUIRE(uni.differences() == 0);
  }

  SECTION("parse-invalid")
  {
    wex::factory::unified_diff uni(
      "diff --git CHANGELOG.md CHANGELOG.md\n"
      "index e4a9c0522..b2637803a 100644\n"
      "--- CHANGELOG.md\n"
      "+++ CHANGELOG.md\n"
      "@ -15,0 +16 @@ The format is based on ...\n");

    wex::factory::unified_diff_parser parser(&uni);

    REQUIRE(!parser.parse());
  }

  SECTION("parse")
  {
    // clang-format off
    wex::factory::unified_diff uni(
      "--- include/wex/factory/unified-diff1.h	2026-01-03 19:27:31\n"
      "+++ include/wex/factory/unified-diff2.h	2026-01-03 19:27:31\n"
      "@@ -109,2 +108,0 @@ private:\n"
      "-  bool parse_header(const std::string& r, const std::string& line, path& p);\n"
      "-\n"
      "--- test/factory/test-unified-diff.cpp\n"
      "+++ test/factory/test-unified-diff.cpp\n"
      "@@ -39 +38,0 @@ TEST_CASE(\"wex::factory::unified_diff\")\n"
      "-    REQUIRE_CALL(uni, report_diff_finish());\n"
      "@@ -43 +42 @@ TEST_CASE(\"wex::factory::unified_diff\")\n"
      "-    REQUIRE(uni.parse());\n"
      "+    REQUIRE(!uni.parse());\n");
    // clang-format on

    wex::factory::unified_diff_parser parser(&uni);

    REQUIRE(parser.parse());
    REQUIRE(uni.range_from_start() == 43);
    REQUIRE(uni.range_from_count() == 1);
    REQUIRE(uni.range_to_start() == 42);
    REQUIRE(uni.range_to_count() == 1);
    REQUIRE(uni.path_from().string() == "include/wex/factory/unified-diff1.h");
    REQUIRE(uni.path_to().string() == "include/wex/factory/unified-diff2.h");
    REQUIRE(
      uni.report_path().string() == "include/wex/factory/unified-diff2.h");
    REQUIRE(uni.differences() == 4);
    REQUIRE(uni.text_added().size() == 1);
    REQUIRE(uni.text_removed().size() == 1);
  }
}
