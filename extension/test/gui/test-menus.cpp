////////////////////////////////////////////////////////////////////////////////
// Name:      test-menus.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/menus.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/menucommands.h>
#include "test.h"

TEST_CASE("wxExMenus", "[menu]")
{
  wxExMenuCommands <wxExMenuCommand > menucommands("test");

  wxXmlNode xml(wxXML_ELEMENT_NODE, "menus");
  xml.AddAttribute("name", "fold.comment");
  new wxXmlNode(&xml, wxXML_TEXT_NODE , "","2");
  
  wxExMenu* menu = new wxExMenu;
  REQUIRE(!wxExMenus::BuildMenu(menucommands.GetCommands(), 500, menu, false));

  std::vector < wxExMenuCommand > commands;
  REQUIRE(!wxExMenus::AddCommands(&xml, commands));
  
  REQUIRE( wxExMenus::GetFileName().IsOk());

  std::vector < wxExMenuCommands <wxExMenuCommand > > entries;
  REQUIRE( wxExMenus::Load("debug", entries));
  REQUIRE(!wxExMenus::Load("xxx", entries));
}
