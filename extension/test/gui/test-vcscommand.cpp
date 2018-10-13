////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs_command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcscommand.h>
#include "test.h"

TEST_CASE("wex::vcs_command")
{
  const wex::vcs_command add("a&dd");
  const wex::vcs_command blame("blame");
  const wex::vcs_command co("checkou&t");
  const wex::vcs_command commit("commit", "main");
  const wex::vcs_command diff("diff", "popup", "submenu");
  const wex::vcs_command log("log", "main");
  const wex::vcs_command help("h&elp", "error", "", "m&e");
  const wex::vcs_command none;

  REQUIRE(add.GetCommand() == "add");
  REQUIRE(add.GetCommand(wex::COMMAND_INCLUDE_SUBCOMMAND | wex::COMMAND_INCLUDE_ACCELL) == "a&dd");
  
  REQUIRE(add.IsAdd());
  REQUIRE(blame.IsBlame());
  REQUIRE(co.IsCheckout());
  REQUIRE(commit.IsCommit());
  REQUIRE(diff.IsDiff());
  REQUIRE(help.IsHelp());
  REQUIRE(log.IsHistory());
  REQUIRE(blame.IsOpen());
  REQUIRE(!help.AskFlags());
  REQUIRE(help.UseSubcommand());

  REQUIRE(add.GetSubMenu().empty());
  REQUIRE(diff.GetSubMenu() == "submenu");
  REQUIRE(help.GetSubMenu() == "m&e");
}
