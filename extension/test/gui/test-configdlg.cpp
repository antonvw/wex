////////////////////////////////////////////////////////////////////////////////
// Name:      test-configdlg.cpp
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

void fixture::testConfigDialog()
{
  wxExItem::UseConfig(true);
  
  wxExItemDialog* dlg = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem> {
      wxExItem("string1", "test1"),
      wxExItem("string2", "test2"),
      wxExItem("string3", "test3"),
      wxExItem("string4", "test4"),
      wxExItem("string5", "test5")},
    "config dialog", 0, 1, wxOK | wxCANCEL | wxAPPLY);
  dlg->Show();
  
  CPPUNIT_ASSERT(dlg->GetItemValue("string1").As<wxString>().empty());
  
  dlg->Reload();
  
  wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxCANCEL));
  
  // Test config dialog without pages.
  wxExItemDialog* dlg1 = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem> {
      wxExItem("string1"),
      wxExItem("string2")},
    "no pages", 0, 1, wxOK | wxCANCEL | wxAPPLY);
  dlg1->Show();
  
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxOK));
  
  // Test config dialog without items.
  wxExItemDialog* dlg2 = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem>(),
    "no items");
  dlg2->Show();
  
  // Test config dialog with empty items.
  wxExItemDialog* dlg3 = new wxExItemDialog(m_Frame, 
    std::vector <wxExItem> {
      wxExItem(), wxExItem(), wxExItem()},
    "empty items");
  dlg3->Show();
}
