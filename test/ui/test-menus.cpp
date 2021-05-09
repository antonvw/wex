////////////////////////////////////////////////////////////////////////////////
// Name:      test-menus.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/menu-command.h>
#include <wex/menu-commands.h>
#include <wex/menus.h>

TEST_CASE("wex::menus")
{
  wex::menu_commands<wex::menu_command> menucommands("test");

  pugi::xml_document doc;
  REQUIRE(doc.load_string("<menus name = \"fold.comment\">2</menus>"));

  auto* menu = new wex::menu;
  REQUIRE(!wex::menus::build_menu(menucommands.get_commands(), 500, menu));

  std::vector<wex::menu_command> commands;
  REQUIRE(!wex::menus::add_commands(doc, commands));

  REQUIRE(!wex::menus::path().empty());

  std::vector<wex::menu_commands<wex::menu_command>> entries;
  REQUIRE(wex::menus::load("debug", entries));
  REQUIRE(!wex::menus::load("xxx", entries));
}
