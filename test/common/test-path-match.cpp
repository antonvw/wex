////////////////////////////////////////////////////////////////////////////////
// Name:      test-path-match.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/path-match.h>
#include <wex/core/config.h>
#include <wex/test/test.h>

TEST_CASE("wex::path_match")
{
  wex::path p("xxx");

  SECTION("constructor-path")
  {
    wex::path_match m(p);

    REQUIRE(m.line().empty());
    REQUIRE(m.context().empty());
    REQUIRE(m.path() == p);
    REQUIRE(m.line_no() == 0);
    REQUIRE(m.pos() == -1);
    REQUIRE(m.tool().id() == wex::ID_LOWEST);
  }

  SECTION("constructor-all")
  {
    wex::path_match m(p, wex::tool(wex::ID_TOOL_REPORT_FIND), "hello", 7, 8);

    REQUIRE(m.context() == "hello");
    REQUIRE(m.line() == "hello");
    REQUIRE(m.path() == p);
    REQUIRE(m.line_no() == 7);
    REQUIRE(m.pos() == 8);
    REQUIRE(m.tool().id() == wex::ID_TOOL_REPORT_FIND);
  }

  SECTION("context")
  {
    const std::string text("this is a text, there are words inside the text");

    REQUIRE(
      wex::path_match(p, wex::tool(wex::ID_TOOL_REPORT_FIND), text, 0, 5)
        .context() == "     " + text);

    REQUIRE(
      wex::path_match(p, wex::tool(wex::ID_TOOL_REPORT_FIND), text, 0, 26)
        .context() == "there are words inside the text");

    REQUIRE(
      wex::path_match(p, wex::tool(wex::ID_TOOL_REPORT_FIND), text, 0, 43)
        .context() == "nside the text");

    wex::config(_("list.Context size")).set(3);

    REQUIRE(
      wex::path_match(p, wex::tool(wex::ID_TOOL_REPORT_FIND), text, 0, 26)
        .context() == "re words inside the text");
  }
}
