////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
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

void fixture::testMenu()
{
  wxExMenu* menu = new wxExMenu;
  
  // AppendSeparator
  menu->AppendSeparator();
  menu->AppendSeparator();
  menu->AppendSeparator();
  menu->AppendSeparator();
  CPPUNIT_ASSERT(menu->GetMenuItemCount() == 0);
  
  // Append  
  menu->Append(wxID_SAVE);
  CPPUNIT_ASSERT(menu->GetMenuItemCount() > 0);
  menu->Append(wxID_SAVE, "mysave");
  
  // AppendEdit
  menu->AppendEdit();
  menu->AppendEdit(true);
  
  // AppendPrint
  menu->AppendPrint();
  
  // AppendSubMenu
  menu->AppendSubMenu(new wxMenu("submenu"), "submenu");
  
  // AppendTools
  CPPUNIT_ASSERT( menu->AppendTools());

  // AppendVCS  
  CPPUNIT_ASSERT(!menu->AppendVCS(wxFileName(), false));
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  CPPUNIT_ASSERT( menu->AppendVCS(wxFileName(), false));
  CPPUNIT_ASSERT( menu->AppendVCS(wxGetCwd(), false));

  // GetStyle
  CPPUNIT_ASSERT(menu->GetStyle() == wxExMenu::MENU_DEFAULT);
  
  // SetStyle
  menu->SetStyle(wxExMenu::MENU_IS_READ_ONLY);
  CPPUNIT_ASSERT(menu->GetStyle() == wxExMenu::MENU_IS_READ_ONLY);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menu, "&Menu");
  m_Frame->SetMenuBar(menubar);
  m_Frame->Update();
}
