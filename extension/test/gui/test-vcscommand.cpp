////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcscommand.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcscommand.h>
#include "test.h"

TEST_CASE("wxExVCSCommand")
{
  const wxExVCSCommand add("a&dd");
  const wxExVCSCommand blame("blame");
  const wxExVCSCommand co("checkou&t");
  const wxExVCSCommand commit("commit", "main");
  const wxExVCSCommand diff("diff", "popup", "submenu");
  const wxExVCSCommand log("log", "main");
  const wxExVCSCommand help("h&elp", "error", "", "m&e");
  const wxExVCSCommand update("update");
  const wxExVCSCommand none;

  REQUIRE(add.GetCommand() == "add");
  REQUIRE(add.GetCommand(true, true) == "a&dd");
  
  REQUIRE(add.IsAdd());
  REQUIRE(blame.IsBlame());
  REQUIRE(co.IsCheckout());
  REQUIRE(commit.IsCommit());
  REQUIRE(diff.IsDiff());
  REQUIRE(help.IsHelp());
  REQUIRE(log.IsHistory());
  REQUIRE(blame.IsOpen());
  REQUIRE(update.IsUpdate());
  REQUIRE(!help.UseFlags());
  REQUIRE(help.UseSubcommand());

  REQUIRE(add.GetSubMenu().empty());
  REQUIRE(diff.GetSubMenu() == "submenu");
  REQUIRE(help.GetSubMenu() == "m&e");
}
