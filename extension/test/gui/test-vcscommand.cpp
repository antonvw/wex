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
#include <wex/vcscommand.h>
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

  REQUIRE(add.get_command() == "add");
  REQUIRE(add.get_command(wex::menu_command::include_t().
    set(wex::menu_command::INCLUDE_SUBCOMMAND).
    set(wex::menu_command::INCLUDE_ACCELL)) == "a&dd");
  
  REQUIRE(add.is_add());
  REQUIRE(blame.is_blame());
  REQUIRE(co.is_checkout());
  REQUIRE(commit.is_commit());
  REQUIRE(diff.is_diff());
  REQUIRE(help.is_help());
  REQUIRE(log.is_history());
  REQUIRE(blame.is_open());
  REQUIRE(!help.ask_flags());
  REQUIRE(help.use_subcommand());

  REQUIRE(add.get_submenu().empty());
  REQUIRE(diff.get_submenu() == "submenu");
  REQUIRE(help.get_submenu() == "m&e");
}
