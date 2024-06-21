////////////////////////////////////////////////////////////////////////////////
// Name:      test-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/mode.h>
#include <wex/vi/vi.h>

#include "../ex/test.h"
#include "test.h"

TEST_CASE("wex::vi_mode")
{
  auto*        vi = new wex::vi(get_stc());
  wex::vi_mode mode(vi);
  std::string  command("x");

  REQUIRE(!mode.insert_commands().empty());

  SUBCASE("command")
  {
    REQUIRE(mode.is_command());
    REQUIRE(!mode.transition(command));
    command = "y";
    REQUIRE(!mode.transition(command));
    REQUIRE(mode.str().empty());
  }

  SUBCASE("insert")
  {
    command = "i";
    REQUIRE(mode.transition(command));
    REQUIRE(mode.is_insert());
    command = "i";
    REQUIRE(mode.transition(command));
    REQUIRE(mode.is_insert());
    REQUIRE(mode.get() == wex::vi_mode::state_t::INSERT);
    REQUIRE(mode.str() == "insert");
    REQUIRE(mode.escape());
    REQUIRE(mode.is_command());
    mode.command();
    REQUIRE(mode.is_command());
    REQUIRE(mode.get() == wex::vi_mode::state_t::COMMAND);

    command = "cc";
    REQUIRE(mode.transition(command));
    REQUIRE(mode.is_insert());
    REQUIRE(mode.escape());
    REQUIRE(mode.is_command());

    get_stc()->SetReadOnly(true);
    command = "i";
    REQUIRE(mode.transition(command));
    REQUIRE(!mode.is_insert());
    REQUIRE(mode.is_command());
    get_stc()->SetReadOnly(false);
  }

  SUBCASE("visuals")
  {
    mode.visual();
    REQUIRE(mode.is_visual());
    mode.visual();
    REQUIRE(mode.is_visual());
    REQUIRE(mode.escape());
    REQUIRE(mode.is_command());

    for (const auto& visual : visuals())
    {
      std::string command(visual.first);
      REQUIRE(mode.transition(command));
      REQUIRE(mode.get() == visual.second);
      command = visual.first;
      REQUIRE(mode.transition(command)); // ignore
      REQUIRE(mode.get() == visual.second);
      REQUIRE(mode.escape());
      REQUIRE(mode.is_command());
    }
  }
}
