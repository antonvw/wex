////////////////////////////////////////////////////////////////////////////////
// Name:      test-command-parser-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/command-parser-data.h>

#include "test.h"

TEST_CASE("wex::command_parser_data")
{
  SECTION("default-constructor")
  {
    wex::command_parser_data pd;

    REQUIRE(pd.command().empty());
    REQUIRE(pd.range().empty());
    REQUIRE(pd.text().empty());
    REQUIRE(!pd.is_global_skip());
  }

  SECTION("constructor")
  {
    wex::command_parser_data pd("m");

    REQUIRE(pd.command().empty());
    REQUIRE(pd.range().empty());
    REQUIRE(pd.text() == "m");
    REQUIRE(!pd.is_global_skip());
  }
}
