////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi-mode.h>
#include <wex/managed-frame.h>
#include <wex/stc.h>
#include <wex/vi.h>
#include "../test.h"

TEST_SUITE_BEGIN("wex::vi");

TEST_CASE("wex::vi_mode")
{
  wex::vi_mode mode(&get_stc()->get_vi());
  
  REQUIRE(!mode.insert_commands().empty());

  // normal
  REQUIRE( mode.normal());
  std::string command("x");
  REQUIRE(!mode.transition(command));
  command = "y";
  REQUIRE(!mode.transition(command));
  REQUIRE( mode.str().empty());
  
  // insert
  command = "i";
  REQUIRE( mode.transition(command));
  REQUIRE( mode.insert());
  command = "i";
  REQUIRE( mode.transition(command));
  REQUIRE( mode.insert());
  REQUIRE( mode.get() == wex::vi_mode::state_t::INSERT);
  REQUIRE( mode.str() == "insert");
  REQUIRE( mode.escape());
  REQUIRE( mode.normal());
  REQUIRE( mode.get() == wex::vi_mode::state_t::NORMAL);

  command = "cc";
  REQUIRE( mode.transition(command));
  REQUIRE( mode.insert());
  REQUIRE( mode.escape());
  REQUIRE( mode.normal());
  
  get_stc()->SetReadOnly(true);
  command = "i";
  REQUIRE( mode.transition(command));
  REQUIRE( mode.normal());
  get_stc()->SetReadOnly(false);
  
  for (const auto& visual : std::vector<std::pair<std::string, wex::vi_mode::state_t>> {
    {"v",wex::vi_mode::state_t::VISUAL},
    {"V",wex::vi_mode::state_t::VISUAL_LINE},
    {"K",wex::vi_mode::state_t::VISUAL_BLOCK}})
  {
    std::string command(visual.first);
    REQUIRE( mode.transition(command));
    REQUIRE( mode.get() == visual.second);
    command = visual.first;
    REQUIRE( mode.transition(command)); // ignore
    REQUIRE( mode.get() == visual.second);
    REQUIRE( mode.escape());
    REQUIRE( mode.normal());
  }
}

TEST_SUITE_END();
