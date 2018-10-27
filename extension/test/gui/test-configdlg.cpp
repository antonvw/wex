////////////////////////////////////////////////////////////////////////////////
// Name:      test-configdlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/itemdlg.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::config_dialog")
{
  wex::item::UseConfig(true);
  
  wex::item_dialog* dlg = new wex::item_dialog({
      {"string1", "test1"},
      {"string2", "test2"},
      {"string3", "test3"},
      {"string4", "test4"},
      {"string5", "test5"}},
    wex::window_data().Button(wxOK | wxCANCEL | wxAPPLY));
  dlg->Show();
  
  REQUIRE( std::any_cast<std::string>(dlg->GetItemValue("string1")).empty());
  
  dlg->Reload();
  
  wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxCANCEL));
  
  // Test config dialog without pages.
  wex::item_dialog* dlg1 = new wex::item_dialog({
      {"string1"},
      {"string2"}},
    wex::window_data().Button(wxOK | wxCANCEL | wxAPPLY));
  dlg1->Show();
  
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxOK));
  
  // Test config dialog without items.
  wex::item_dialog* dlg2 = new wex::item_dialog(
    std::vector <wex::item>());
  dlg2->Show();
  
  // Test config dialog with empty items.
  wex::item_dialog* dlg3 = new wex::item_dialog(
    std::vector <wex::item> {{}, {}, {}});
  dlg3->Show();
}
