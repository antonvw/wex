////////////////////////////////////////////////////////////////////////////////
// Name:      test-configdlg.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExConfigDialog")
{
  wxExItem::UseConfig(true);
  
  wxExItemDialog* dlg = new wxExItemDialog(GetFrame(), 
    std::vector <wxExItem> {
      {"string1", "test1"},
      {"string2", "test2"},
      {"string3", "test3"},
      {"string4", "test4"},
      {"string5", "test5"}},
    "config dialog", 0, 1, wxOK | wxCANCEL | wxAPPLY);
  dlg->Show();
  
  REQUIRE(dlg->GetItemValue("string1").As<wxString>().empty());
  
  dlg->Reload();
  
  wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxCANCEL));
  
  // Test config dialog without pages.
  wxExItemDialog* dlg1 = new wxExItemDialog(GetFrame(), 
    std::vector <wxExItem> {
      {"string1"},
      {"string2"}},
    "no pages", 0, 1, wxOK | wxCANCEL | wxAPPLY);
  dlg1->Show();
  
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxOK));
  
  // Test config dialog without items.
  wxExItemDialog* dlg2 = new wxExItemDialog(GetFrame(), 
    std::vector <wxExItem>(),
    "no items");
  dlg2->Show();
  
  // Test config dialog with empty items.
  wxExItemDialog* dlg3 = new wxExItemDialog(GetFrame(), 
    std::vector <wxExItem> {{}, {}, {}},
    "empty items");
  dlg3->Show();
}
