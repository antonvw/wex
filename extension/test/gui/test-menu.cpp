////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include "test.h"

TEST_CASE("wxExMenu", "[menu]")
{
  wxExMenu* menu = new wxExMenu;
  
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
  menu->AppendVCS(wxFileName(), false);
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  REQUIRE( menu->AppendVCS(wxFileName(), false));
  REQUIRE( menu->AppendVCS(wxGetCwd(), false));

  // GetStyle
  REQUIRE(menu->GetStyle() == wxExMenu::MENU_DEFAULT);
  
  // SetStyle
  menu->SetStyle(wxExMenu::MENU_IS_READ_ONLY);
  REQUIRE(menu->GetStyle() == wxExMenu::MENU_IS_READ_ONLY);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menu, "&Menu");
  GetFrame()->SetMenuBar(menubar);
  GetFrame()->Update();
}
