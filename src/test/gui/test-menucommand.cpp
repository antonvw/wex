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

  REQUIRE(add.get_command() == "add");
  REQUIRE(add.get_command(wex::menu_command::include_t().set(
    wex::menu_command::INCLUDE_SUBCOMMAND).set(
    wex::menu_command::INCLUDE_ACCELL)) == "a&dd");
  
  REQUIRE(help.get_command() == "help me");
  REQUIRE(help.get_command(wex::menu_command::include_t().set(
    wex::menu_command::INCLUDE_SUBCOMMAND).set(
    wex::menu_command::INCLUDE_ACCELL)) == "h&elp m&e");
  REQUIRE(help.get_command(wex::menu_command::include_t().set(
    wex::menu_command::INCLUDE_ACCELL)) == "h&elp");
  
  REQUIRE( add.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( add.type().test(wex::menu_command::IS_POPUP));
  REQUIRE( blame.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( blame.type().test(wex::menu_command::IS_POPUP));
  REQUIRE( commit.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( diff.type().test(wex::menu_command::IS_POPUP));
  REQUIRE( help.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( help.type().test(wex::menu_command::IS_POPUP));

  REQUIRE(!help.ask_flags());
  REQUIRE( help.is_help());
  REQUIRE( help.use_subcommand());

  REQUIRE( add.get_submenu().empty());
  REQUIRE( diff.get_submenu() == "submenu");
  REQUIRE( help.get_submenu() == "m&e");

  REQUIRE(none.type().none());
}
