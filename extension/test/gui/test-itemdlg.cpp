////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemdlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/itemdlg.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExItemDialog")
{
#if wxCHECK_VERSION(3,1,0)
  wxExItem::UseConfig(false);
  
  // Test dialog without pages.
  wxExItemDialog* dlg1 = new wxExItemDialog({
      {"string1", "hello1"},
      {"string2", "hello2"},
      {"int1", ITEM_TEXTCTRL_INT, wxString("10")},
      {"int2", ITEM_TEXTCTRL_INT, wxString("20")},
      {"float1", ITEM_TEXTCTRL_FLOAT, wxString("20.0")}},
    wxExWindowData().Button(wxOK | wxCANCEL | wxAPPLY));
  dlg1->Show();
  dlg1->ForceCheckBoxChecked();
  
  dlg1->Reload();
  dlg1->WriteAllItems(std::cout);
  
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  
  REQUIRE(std::any_cast<long>(dlg1->GetItemValue("int1")) == 10l);
  REQUIRE(std::any_cast<long>(dlg1->GetItemValue("int2")) == 20l);
  REQUIRE(std::any_cast<double>(dlg1->GetItemValue("float1")) == 20.0);
  
  // Test dialog without items.
  wxExItemDialog* dlg2 = new wxExItemDialog(std::vector <wxExItem>());
  dlg2->Show();
  
  // Test dialog with empty items.
  wxExItemDialog* dlg3 = new wxExItemDialog({{}, {}, {}}, wxExWindowData());
  dlg3->Show();
#endif
}
