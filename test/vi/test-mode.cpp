////////////////////////////////////////////////////////////////////////////////
// Name:      test-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/mode.h>
#include <wex/vi/vi.h>

#include "../ex/test.h"
#include "test.h"

TEST_CASE("wex::vi_mode")
{
  auto*        vi = new wex::vi(get_stc());
  wex::vi_mode mode(vi);

  REQUIRE(!mode.insert_commands().empty());

  // command
  REQUIRE(mode.is_command());
  std::string command("x");
  REQUIRE(!mode.transition(command));
  command = "y";
  REQUIRE(!mode.transition(command));
  REQUIRE(mode.str().empty());

  // insert
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
  REQUIRE(mode.is_command());
  get_stc()->SetReadOnly(false);

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
