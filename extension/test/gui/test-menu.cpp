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
#include <wex/config.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include "test.h"

TEST_CASE("wex::menu")
{
  wex::menu* menu = new wex::menu;
  
  // append_separator
  menu->append_separator();
  menu->append_separator();
  menu->append_separator();
  menu->append_separator();
  REQUIRE(menu->GetMenuItemCount() == 0);
  
  // Append  
  menu->Append(wxID_SAVE);
  REQUIRE(menu->GetMenuItemCount() > 0);
  menu->Append(wxID_SAVE, "mysave");
  
  // append_edit
  menu->append_edit();
  menu->append_edit(true);
  
  // append_print
  menu->append_print();
  
  // append_submenu
  menu->append_submenu(new wxMenu("submenu"), "submenu");
  
  // append_tools
  REQUIRE( menu->append_tools());

  // append_vcs  
  menu->append_vcs(wex::path(), false);
  wex::config(_("Base folder")).set(wxGetCwd().ToStdString());
  REQUIRE( menu->append_vcs(wex::path(), false));
  REQUIRE( menu->append_vcs(wex::path::Current(), false));

  // style
  REQUIRE(menu->style() == wex::menu::DEFAULT);
  
  // style
  menu->style(wex::menu::IS_READ_ONLY);
  REQUIRE(menu->style() == wex::menu::IS_READ_ONLY);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menu, "&Menu");
  GetFrame()->SetMenuBar(menubar);
  GetFrame()->Update();
}
