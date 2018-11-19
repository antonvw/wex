////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemdlg.cpp
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

TEST_CASE("wex::item_dialog")
{
#if wxCHECK_VERSION(3,1,0)
  wex::item::use_config(false);
  
  // Test dialog without pages.
  wex::item_dialog* dlg1 = new wex::item_dialog({
      {"string1", "hello1"},
      {"string2", "hello2"},
      {"int1", wex::item::TEXTCTRL_INT, std::string("10")},
      {"int2", wex::item::TEXTCTRL_INT, std::string("20")},
      {"float1", wex::item::TEXTCTRL_FLOAT, std::string("20.0")}},
    wex::window_data().button(wxOK | wxCANCEL | wxAPPLY));

  dlg1->Show();
  dlg1->force_checkbox_checked();
  dlg1->reload();

#ifdef DEBUG
  for (const auto& i : dlg1->get_items())
  { 
    std::cout << i.Log().str();
  };
#endif
  
  wxPostEvent(dlg1, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  
  REQUIRE(std::any_cast<long>(dlg1->get_item_value(std::string("int1"))) == 10l);
  REQUIRE(std::any_cast<long>(dlg1->get_item_value(std::string("int2"))) == 20l);
  REQUIRE(std::any_cast<double>(dlg1->get_item_value(std::string("float1"))) == 20.0);
  
  // Test dialog without items.
  wex::item_dialog* dlg2 = new wex::item_dialog(std::vector <wex::item>());
  dlg2->Show();
  
  // Test dialog with empty items.
  wex::item_dialog* dlg3 = new wex::item_dialog({{}, {}, {}}, wex::window_data());
  dlg3->Show();
#endif
}
