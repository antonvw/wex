////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include "test.h"

TEST_CASE("wex::menu")
{
  wex::menu* menu = new wex::menu;
  
  // AppendSeparator
  menu->AppendSeparator();
  menu->AppendSeparator();
  menu->AppendSeparator();
  menu->AppendSeparator();
  REQUIRE(menu->GetMenuItemCount() == 0);
  
  // Append  
  menu->Append(wxID_SAVE);
  REQUIRE(menu->GetMenuItemCount() > 0);
  menu->Append(wxID_SAVE, "mysave");
  
  // AppendEdit
  menu->AppendEdit();
  menu->AppendEdit(true);
  
  // AppendPrint
  menu->AppendPrint();
  
  // AppendSubMenu
  menu->AppendSubMenu(new wxMenu("submenu"), "submenu");
  
  // AppendTools
  REQUIRE( menu->AppendTools());

  // AppendVCS  
  menu->AppendVCS(wex::path(), false);
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  REQUIRE( menu->AppendVCS(wex::path(), false));
  REQUIRE( menu->AppendVCS(wex::path::Current(), false));

  // GetStyle
  REQUIRE(menu->GetStyle() == wex::menu::MENU_DEFAULT);
  
  // SetStyle
  menu->SetStyle(wex::menu::MENU_IS_READ_ONLY);
  REQUIRE(menu->GetStyle() == wex::menu::MENU_IS_READ_ONLY);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menu, "&Menu");
  GetFrame()->SetMenuBar(menubar);
  GetFrame()->Update();
}
