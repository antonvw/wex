////////////////////////////////////////////////////////////////////////////////
// Name:      test-command-parser.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/command-parser.h>

#include "test.h"

TEST_CASE("wex::command_parser")
{
  auto* stc = get_stc();
  stc->set_text("hello\nhello1\nhello2");
  auto* ex = new wex::ex(stc);

  SUBCASE("empty-command")
  {
    wex::command_parser cp(ex);

    REQUIRE(!cp.is_ok());
    REQUIRE(cp.command().empty());
    REQUIRE(cp.range().empty());
    REQUIRE(cp.text().empty());
    REQUIRE(cp.type() == wex::command_parser::address_t::NO_ADDR);
  }

  SUBCASE("no-addr")
  {
    wex::command_parser cp(ex, "10");

    REQUIRE(!cp.is_ok());
    REQUIRE(cp.command().empty());
    REQUIRE(cp.range().empty());
    REQUIRE(cp.text() == "10");
    REQUIRE(cp.type() == wex::command_parser::address_t::NO_ADDR);
  }

  SUBCASE("one-addr")
  {
    wex::command_parser cp(ex, ".ma z");

    REQUIRE(cp.is_ok());
    REQUIRE(cp.command() == "ma");
    REQUIRE(cp.range() == ".");
    REQUIRE(cp.text() == "z");
    REQUIRE(cp.type() == wex::command_parser::address_t::ONE_ADDR);
  }

  SUBCASE("two-addr")
  {
    wex::command_parser cp(ex, "1,3y");

    REQUIRE(cp.is_ok());
    REQUIRE(cp.command() == "y");
    REQUIRE(cp.range() == "1,3");
    REQUIRE(cp.text().empty());
    REQUIRE(cp.type() == wex::command_parser::address_t::TWO_ADDR);
  }
}
