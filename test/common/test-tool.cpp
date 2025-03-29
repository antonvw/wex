////////////////////////////////////////////////////////////////////////////////
// Name:      test-tool.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/statistics.h>
#include <wex/common/tool.h>
#include <wex/core/log-none.h>
#include <wex/test/test.h>

TEST_CASE("wex::tool")
{
  SECTION("default-constructor")
  {
    wex::tool tool;
    REQUIRE(tool.id() == wex::ID_LOWEST);

    wex::log_none off;
    REQUIRE(tool.info().empty());
  }

  SECTION("constructor")
  {
    wex::tool tool(wex::ID_ALL_CLOSE);
    wex::tool::add_info(wex::ID_ALL_CLOSE, "this is ok");

    wex::statistics<int> stat;

    REQUIRE(tool.id() == wex::ID_ALL_CLOSE);
    REQUIRE(!tool.get_tool_info().empty());
    REQUIRE(tool.info(&stat) == "this is ok 0 file(s)");
    REQUIRE(!tool.is_find_type());

    REQUIRE(!wex::tool(wex::ID_TOOL_REPORT_FIND).info().empty());

    REQUIRE(wex::tool(wex::ID_TOOL_REPORT_FIND).is_find_type());
    REQUIRE(wex::tool(wex::ID_TOOL_REPLACE).is_find_type());
  }
}

TEST_CASE("wex::tool_info")
{
  SECTION("default-constructor")
  {
    wex::tool_info info;
    REQUIRE(info.help_text().empty());
    REQUIRE(info.info().empty());
    REQUIRE(info.text().empty());
  }

  SECTION("constructor")
  {
    wex::tool_info info("x", "y", "z");
    REQUIRE(info.help_text() == "z");
    REQUIRE(info.info() == "x");
    REQUIRE(info.text() == "y");
  }
}
