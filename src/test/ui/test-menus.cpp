////////////////////////////////////////////////////////////////////////////////
// Name:      test-menus.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/menus.h>
#include <wex/menucommand.h>
#include <wex/menucommands.h>
#include "../test.h"

TEST_CASE("wex::menus")
{
  wex::menu_commands <wex::menu_command > menucommands("test");

  pugi::xml_document doc;
  REQUIRE( doc.load_string("<menus name = \"fold.comment\">2</menus>"));

  auto* menu = new wex::menu;
  REQUIRE(!wex::menus::build_menu(menucommands.get_commands(), 500, menu));

  std::vector < wex::menu_command > commands;
  REQUIRE(!wex::menus::add_commands(doc, commands));
  
  REQUIRE(!wex::menus::get_filename().empty());

  std::vector < wex::menu_commands <wex::menu_command > > entries;
  REQUIRE( wex::menus::load("debug", entries));
  REQUIRE(!wex::menus::load("xxx", entries));
}
