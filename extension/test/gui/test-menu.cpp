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
#include <wx/extension/menu.h>
#include "test.h"

void wxExGuiTestFixture::testMenu()
{
  wxExMenu menu;
  
  menu.AppendSeparator();
  menu.AppendSeparator();
  menu.AppendSeparator();
  menu.AppendSeparator();
  CPPUNIT_ASSERT(menu.GetMenuItemCount() == 0);
  
  menu.AppendBars();
  CPPUNIT_ASSERT(menu.GetMenuItemCount() > 0);
  
  menu.Append(wxID_SAVE);
  menu.Append(wxID_SAVE, "mysave");
  menu.AppendEdit();
  menu.AppendEdit(true);
  menu.AppendPrint();
  
  wxMenu* submenu = new wxMenu("submenu");
  menu.AppendSubMenu(submenu, "submenu");
  CPPUNIT_ASSERT(menu.AppendTools());
  menu.AppendVCS(wxFileName(), false); // see alo testVCS
}
