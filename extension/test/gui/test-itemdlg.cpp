////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemdlg.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/itemdlg.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testItemDialog()
{
#if wxCHECK_VERSION(3,1,0)
  wxExItem::UseConfig(false);
  
  // Test dialog without pages.
  wxExItemDialog* dlg1 = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem> {
      wxExItem("string1", "hello1"),
      wxExItem("string2", "hello2"),
      wxExItem("int1", ITEM_TEXTCTRL_INT, "10"),
      wxExItem("int2", ITEM_TEXTCTRL_INT, "20"),
      wxExItem("float1", ITEM_TEXTCTRL_FLOAT, "20.0")},
    "no pages", 0, 1, wxOK | wxCANCEL | wxAPPLY);
  dlg1->Show();
  dlg1->ForceCheckBoxChecked();
  
  dlg1->Reload();
  
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  
  CPPUNIT_ASSERT(dlg1->GetItemValue("int1") == 10);
  CPPUNIT_ASSERT(dlg1->GetItemValue("int2") == 20);
  CPPUNIT_ASSERT(dlg1->GetItemValue("float1") == 20.0);
  
  // Test dialog without items.
  wxExItemDialog* dlg2 = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem>(),
    "no items");
  dlg2->Show();
  
  // Test dialog with empty items.
  wxExItemDialog* dlg3 = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem> {
      wxExItem(), wxExItem(), wxExItem()},
    "empty items");
  dlg3->Show();
#endif
}
