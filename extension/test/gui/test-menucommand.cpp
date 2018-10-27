////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu_command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/menucommand.h>
#include "test.h"

TEST_CASE("wex::menu_command")
{
  const wex::menu_command add("a&dd");
  const wex::menu_command blame("blame");
  const wex::menu_command co("checkou&t");
  const wex::menu_command commit("commit", "main");
  const wex::menu_command diff("diff", "popup", "submenu");
  const wex::menu_command log("log", "main");
  const wex::menu_command help("h&elp", "", "", "m&e");
  const wex::menu_command update("update");
  const wex::menu_command none;

  REQUIRE(add.GetCommand() == "add");
  REQUIRE(add.GetCommand(wex::menu_command::INCLUDE_SUBCOMMAND | wex::menu_command::INCLUDE_ACCELL) == "a&dd");
  REQUIRE(help.GetCommand() == "help me");
  REQUIRE(help.GetCommand(wex::menu_command::INCLUDE_SUBCOMMAND | wex::menu_command::INCLUDE_ACCELL) == "h&elp m&e");
  REQUIRE(help.GetCommand(wex::menu_command::INCLUDE_ACCELL) == "h&elp");
  REQUIRE(help.GetCommand(wex::menu_command::INCLUDE_NONE) == "help");
  
  REQUIRE((add.GetType() & wex::menu_command::IS_MAIN) > 0);
  REQUIRE((add.GetType() & wex::menu_command::IS_POPUP) > 0);
  REQUIRE((blame.GetType() & wex::menu_command::IS_MAIN) > 0);
  REQUIRE((blame.GetType() & wex::menu_command::IS_POPUP) > 0);
  REQUIRE((commit.GetType() & wex::menu_command::IS_MAIN) > 0);
  REQUIRE((diff.GetType() & wex::menu_command::IS_POPUP) > 0);
  REQUIRE((help.GetType() & wex::menu_command::IS_MAIN) > 0);
  REQUIRE((help.GetType() & wex::menu_command::IS_POPUP) > 0);

  REQUIRE(!help.AskFlags());
  REQUIRE( help.IsHelp());
  REQUIRE( help.UseSubcommand());

  REQUIRE(add.GetSubMenu().empty());
  REQUIRE(diff.GetSubMenu() == "submenu");
  REQUIRE(help.GetSubMenu() == "m&e");

  REQUIRE(none.GetType() == wex::menu_command::IS_NONE);
}
