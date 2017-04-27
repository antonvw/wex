////////////////////////////////////////////////////////////////////////////////
// Name:      test-menus.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/menus.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/menucommands.h>
#include "test.h"

TEST_CASE("wxExMenus")
{
  wxExMenuCommands <wxExMenuCommand > menucommands("test");

  pugi::xml_document doc;
  REQUIRE( doc.load_string("<menus name = \"fold.comment\">2</menus>"));

  wxExMenu* menu = new wxExMenu;
  REQUIRE(!wxExMenus::BuildMenu(menucommands.GetCommands(), 500, menu, false));

  std::vector < wxExMenuCommand > commands;
  REQUIRE(!wxExMenus::AddCommands(doc, commands));
  
  REQUIRE(!wxExMenus::GetFileName().GetFullPath().empty());

  std::vector < wxExMenuCommands <wxExMenuCommand > > entries;
  REQUIRE( wxExMenus::Load("debug", entries));
  REQUIRE(!wxExMenus::Load("xxx", entries));
}
