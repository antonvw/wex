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
  REQUIRE(add.GetCommand(true, true) == "a&dd");
  REQUIRE(help.GetCommand() == "help me");
  REQUIRE(help.GetCommand(true, true) == "h&elp m&e");
  REQUIRE(help.GetCommand(false, true) == "h&elp");
  REQUIRE(help.GetCommand(false, false) == "help");
  
  REQUIRE((add.GetType() & wxExMenuCommand::MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((add.GetType() & wxExMenuCommand::MENU_COMMAND_IS_POPUP) > 0);
  REQUIRE((blame.GetType() & wxExMenuCommand::MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((blame.GetType() & wxExMenuCommand::MENU_COMMAND_IS_POPUP) > 0);
  REQUIRE((commit.GetType() & wxExMenuCommand::MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((diff.GetType() & wxExMenuCommand::MENU_COMMAND_IS_POPUP) > 0);
  REQUIRE((help.GetType() & wxExMenuCommand::MENU_COMMAND_IS_MAIN) > 0);
  REQUIRE((help.GetType() & wxExMenuCommand::MENU_COMMAND_IS_POPUP) > 0);

  REQUIRE(!help.AskFlags());
  REQUIRE( help.IsHelp());
  REQUIRE( help.UseSubcommand());

  REQUIRE(add.GetSubMenu().empty());
  REQUIRE(diff.GetSubMenu() == "submenu");
  REQUIRE(help.GetSubMenu() == "m&e");

  REQUIRE(none.GetType() == wxExMenuCommand::MENU_COMMAND_IS_NONE);
}
