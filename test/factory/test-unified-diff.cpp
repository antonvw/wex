////////////////////////////////////////////////////////////////////////////////
// Name:      test-unified-diff.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/factory/unified-diff.h>

#include "test.h"

class mock_unified_diff : public wex::factory::unified_diff
{
public:
  mock_unified_diff(const std::string& input)
    : wex::factory::unified_diff(input)
  {
    ;
  };
  MAKE_MOCK0(report_diff, bool(), override);
  MAKE_MOCK0(report_diff_finish, void(), override);
};

TEST_CASE("wex::factory::unified_diff")
{
  SUBCASE("constructor")
  {
    mock_unified_diff uni("");

    REQUIRE_CALL(uni, report_diff()).RETURN(false);
    REQUIRE_CALL(uni, report_diff_finish());
    REQUIRE(uni.parse());
    REQUIRE(uni.differences() == 0);
    REQUIRE(uni.range_from_count() == 0);
    REQUIRE(uni.range_to_count() == 0);
  }

  SUBCASE("parse-invalid")
  {
    wex::log_none off;
    REQUIRE(!wex::factory::unified_diff("error\n").parse());
    REQUIRE(!wex::factory::unified_diff(
               "diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
               "index a23525b3c..26e9e8fc1 100644\n"
               "--- a/CHANGELOG.md\n"
               "+++ b/CHANGELOG.md\n"
               "@@ -10,0 + @@ The format is based on [Keep a Changelog].\n"
               "+- added git diff option\n")
               .parse());
    REQUIRE(
      !wex::factory::unified_diff("diff --git a/CHANGELOG.md b/CHANGELOG.md\n"
                                  "index a23525b3c..26e9e8fc1 100644\n"
                                  "--- a/CHANGELOG.md\n"
                                  "+++ b/CHANGELOG.md\n"
                                  "+- added git diff option\n")
         .parse());
  }

  SUBCASE("parse-valid")
  {
    mock_unified_diff uni(
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

    REQUIRE(uni.is_first());

    REQUIRE_CALL(uni, report_diff()).RETURN(true).TIMES(AT_LEAST(5));
    REQUIRE_CALL(uni, report_diff_finish());

    const auto res(uni.parse());

    REQUIRE(res);
    REQUIRE(uni.path_from().string() == "CHANGELOG.md");
    REQUIRE(uni.path_to().string() == "CHANGELOG.md");
    REQUIRE(uni.range_from_start() == 38);
    REQUIRE(uni.range_from_count() == 0);
    REQUIRE(uni.range_to_start() == 37);
    REQUIRE(uni.range_to_count() == 1);
    REQUIRE(uni.text_added().front() == "- test");
    REQUIRE(uni.text_removed().empty());
    REQUIRE(!uni.is_first());
  }

  SUBCASE("parse-valid-other")
  {
    mock_unified_diff uni(
      "diff --git a/external/pugixml b/external/pugixml\n"
      "--- a/external/pugixml\n"
      "+++ b/external/pugixml\n"
      "@@ -1 +1 @@\n"
      "-Subproject commit 6909df2478f7eb092e8e5b5cda097616b2595cc6\n"
      "+Subproject commit 6909df2478f7eb092e8e5b5cda097616b2595cc6-dirty\n"
      "diff --git a/external/wxWidgets b/external/wxWidgets\n"
      "--- a/external/wxWidgets\n"
      "+++ b/external/wxWidgets\n"
      "@@ -1 +1 @@\n"
      "-Subproject commit 12b09a5e5ea76a1a0c27b769e821b37d803a4cb7\n"
      "+Subproject commit 12b09a5e5ea76a1a0c27b769e821b37d803a4cb7-dirty\n"
      "diff --git a/include/wex/vcs/unified-diff.h "
      "b/include/wex/vcs/unified-diff.h\n"
      "index 396ed3f8b..3d274e39e 100644\n"
      "--- a/include/wex/vcs/unified-diff.h\n"
      "+++ b/include/wex/vcs/unified-diff.h\n"
      "@@ -42 +42 @@ public:\n"
      "-    vcs_entry* entry,\n"
      "+    const vcs_entry* entry,\n"
      "@@ -85,2 +85,2 @@ private:\n"
      "-  vcs_entry*      m_vcs_entry{nullptr};\n"
      "-  factory::frame* m_frame{nullptr};\n"
      "+  const vcs_entry* m_vcs_entry{nullptr};\n"
      "+  factory::frame*  m_frame{nullptr};\n");

    REQUIRE_CALL(uni, report_diff()).RETURN(true).TIMES(AT_LEAST(5));
    REQUIRE_CALL(uni, report_diff_finish());

    const auto res(uni.parse());
    REQUIRE(res);
    REQUIRE(uni.path_from().string() == "include/wex/vcs/unified-diff.h");
    REQUIRE(uni.path_to().string() == "include/wex/vcs/unified-diff.h");
    REQUIRE(uni.range_from_start() == 85);
    REQUIRE(uni.range_from_count() == 2);
    REQUIRE(uni.range_to_start() == 85);
    REQUIRE(uni.range_to_count() == 2);
  }

  SUBCASE("parse-valid-sub")
  {
    mock_unified_diff uni(
      "diff --git a/external/pugixml b/external/pugixml\n"
      "--- a/external/pugixml\n"
      "+++ b/external/pugixml\n"
      "@@ -1 +1 @@\n"
      "-Subproject commit 6909df2478f7eb092e8e5b5cda097616b2595cc6\n"
      "+Subproject commit 6909df2478f7eb092e8e5b5cda097616b2595cc6-dirty\n"
      "diff --git a/external/wxWidgets b/external/wxWidgets\n"
      "--- a/external/wxWidgets\n"
      "+++ b/external/wxWidgets\n"
      "@@ -1 +1 @@\n"
      "-Subproject commit 12b09a5e5ea76a1a0c27b769e821b37d803a4cb7\n"
      "+Subproject commit 12b09a5e5ea76a1a0c27b769e821b37d803a4cb7-dirty\n");

    ALLOW_CALL(uni, report_diff()).RETURN(true);
    REQUIRE_CALL(uni, report_diff_finish());

    const auto res(uni.parse());
    REQUIRE(res);
    REQUIRE(uni.range_from_start() == 1);
    REQUIRE(uni.range_from_count() == 1);
    REQUIRE(uni.range_to_start() == 1);
    REQUIRE(uni.range_to_count() == 1);
    CAPTURE(uni.path_from().string());
  }
}
