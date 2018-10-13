////////////////////////////////////////////////////////////////////////////////
// Name:      test-menus.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/menus.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/menucommands.h>
#include "test.h"

TEST_CASE("wex::menus")
{
  wex::menu_commands <wex::menu_command > menucommands("test");

  pugi::xml_document doc;
  REQUIRE( doc.load_string("<menus name = \"fold.comment\">2</menus>"));

  wex::menu* menu = new wex::menu;
  REQUIRE(!wex::menus::BuildMenu(menucommands.GetCommands(), 500, menu, false));

  std::vector < wex::menu_command > commands;
  REQUIRE(!wex::menus::AddCommands(doc, commands));
  
  REQUIRE(!wex::menus::GetFileName().Path().empty());

  std::vector < wex::menu_commands <wex::menu_command > > entries;
  REQUIRE( wex::menus::Load("debug", entries));
  REQUIRE(!wex::menus::Load("xxx", entries));
}
