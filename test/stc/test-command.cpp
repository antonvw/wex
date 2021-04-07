////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-command.h>

#include "test.h"

TEST_CASE("wex::ex_command")
{
  auto* stc = get_stc();
  stc->set_text("more text\notherline\nother line");

  wex::ex_command command(stc);

  SUBCASE("append_exec")
  {
    command.append('g');
    REQUIRE(command.command() == "g");
    command.append('g');
    REQUIRE(command.command() == "gg");
    REQUIRE(command.front() == 'g');
    REQUIRE(command.back() == 'g');
    REQUIRE(command.size() == 2);
    command.pop_back();
    REQUIRE(command.size() == 1);
    REQUIRE(command.append_exec('g'));
    REQUIRE(stc->get_current_line() == 0);
  }

  SUBCASE("exec")
  {
    command.set("G");
    REQUIRE(command.command() == "G");
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(command.exec());
    REQUIRE(stc->get_current_line() == 2);
  }
}
