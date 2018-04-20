////////////////////////////////////////////////////////////////////////////////
// Name:      test-menucommand.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/menucommand.h>
#include "test.h"

TEST_CASE("wxExMenuCommand")
{
  const wxExMenuCommand add("a&dd");
  const wxExMenuCommand blame("blame");
  const wxExMenuCommand co("checkou&t");
  const wxExMenuCommand commit("commit", "main");
  const wxExMenuCommand diff("diff", "popup", "submenu");
  const wxExMenuCommand log("log", "main");
  const wxExMenuCommand help("h&elp", "", "", "m&e");
  const wxExMenuCommand update("update");
  const wxExMenuCommand none;

  REQUIRE(add.GetCommand() == "add");
  REQUIRE(add.GetCommand(COMMAND_INCLUDE_SUBCOMMAND | COMMAND_INCLUDE_ACCELL) == "a&dd");
  REQUIRE(help.GetCommand() == "help me");
  REQUIRE(help.GetCommand(COMMAND_INCLUDE_SUBCOMMAND | COMMAND_INCLUDE_ACCELL) == "h&elp m&e");
  REQUIRE(help.GetCommand(COMMAND_INCLUDE_ACCELL) == "h&elp");
  REQUIRE(help.GetCommand(COMMAND_INCLUDE_NONE) == "help");
  
  REQUIRE((add.GetType() & MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((add.GetType() & MENU_COMMAND_IS_POPUP) > 0);
  REQUIRE((blame.GetType() & MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((blame.GetType() & MENU_COMMAND_IS_POPUP) > 0);
  REQUIRE((commit.GetType() & MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((diff.GetType() & MENU_COMMAND_IS_POPUP) > 0);
  REQUIRE((help.GetType() & MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((help.GetType() & MENU_COMMAND_IS_POPUP) > 0);

  REQUIRE(!help.AskFlags());
  REQUIRE( help.IsHelp());
  REQUIRE( help.UseSubcommand());

  REQUIRE(add.GetSubMenu().empty());
  REQUIRE(diff.GetSubMenu() == "submenu");
  REQUIRE(help.GetSubMenu() == "m&e");

  REQUIRE(none.GetType() == MENU_COMMAND_IS_NONE);
}
