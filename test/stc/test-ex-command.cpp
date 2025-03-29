////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/ex-command.h>

#include "test.h"

TEST_CASE("wex::ex_command")
{
  auto* stc = get_stc();
  stc->set_text("more text\notherline\nother line");

  wex::ex_command command(stc);

  SECTION("append_exec")
  {
    REQUIRE(!command.append_exec('g'));
    REQUIRE(command.append_exec('g'));
    REQUIRE(stc->get_current_line() == 0);
  }

  SECTION("exec")
  {
    command.set("G");
    REQUIRE(command.command() == "G");
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(command.exec());
    REQUIRE(stc->get_current_line() == 2);
  }
}
