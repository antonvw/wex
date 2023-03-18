////////////////////////////////////////////////////////////////////////////////
// Name:      test-menus.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/core/menu-command.h>
#include <wex/ui/menu-commands.h>
#include <wex/ui/menus.h>

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
