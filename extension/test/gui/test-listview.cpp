////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/listview.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::listview")
{
  wex::listview* lv = new wex::listview();
  add_pane(frame(), lv);
  
  wex::listview::config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL));
  
  REQUIRE(lv->data().image() == wex::listview_data::IMAGE_ART);
  
  lv->config_get();
  
  wex::column intcol(wex::column("Int", wex::column::INT));
  REQUIRE(!intcol.is_sorted_ascending());
  intcol.set_is_sorted_ascending(wex::SORT_ASCENDING);
  REQUIRE( intcol.is_sorted_ascending());
  
  REQUIRE( lv->append_columns({{intcol}}));
  REQUIRE( lv->append_columns({
    {"Date", wex::column::DATE},
    {"Float", wex::column::FLOAT},
    {"String", wex::column::STRING}}));

  REQUIRE( lv->find_column("Int") == 0);
  REQUIRE( lv->find_column("Date") == 1);
  REQUIRE( lv->find_column("Float") == 2);
  REQUIRE( lv->find_column("String") == 3);
  
  REQUIRE( lv->insert_item({"95"}));
  REQUIRE( lv->insert_item({"95", "", "", "100"}));
  REQUIRE(!lv->insert_item({"test"}));
  REQUIRE(!lv->insert_item({"1", "2", "3", "4", "5"}));
  
  REQUIRE( lv->find_next("95"));
  REQUIRE(!lv->find_next("test"));
  
  REQUIRE(!lv->item_from_text("a new item"));
  REQUIRE(!lv->find_next("another new item"));
  REQUIRE( lv->item_from_text("999"));
  
  REQUIRE( lv->item_to_text(0).find("95") != std::string::npos);
  REQUIRE(!lv->item_to_text(-1).empty());
  
  //lv->print(); // waits for input
  //lv->print_preview();

  // Clears all items, to test sorting later on.  
  lv->clear();
  
  lv->items_update();
  
  for (int i = 0; i < 10; i++)
  {
    REQUIRE( lv->insert_item({
      std::to_string(i),
      get_testpath("test.h").stat().get_modification_time(),
      std::to_string((float)i / 2.0),
      "hello " + std::to_string(i)}));
  }
  
  // Test sorting.
  REQUIRE(!lv->sort_column("xxx"));
  REQUIRE( lv->sort_column("Int", wex::SORT_ASCENDING));
  REQUIRE( lv->get_item_text(0, "Int") == "0");
  REQUIRE( lv->get_item_text(1, "Int") == "1");
  REQUIRE( lv->sort_column("Int", wex::SORT_DESCENDING));
  REQUIRE( lv->get_item_text(0, "Int") == "9");
  REQUIRE( lv->get_item_text(1, "Int") == "8");
  REQUIRE( lv->sort_column("Date"));
  REQUIRE( lv->sort_column("Float"));
  REQUIRE( lv->sort_column("String"));
  
  REQUIRE( lv->sorted_column_no() == 3);
  lv->sort_column_reset();
  REQUIRE( lv->sorted_column_no() == -1);
  
  lv->SetItem(0, 1, "incorrect date"); // return value on osx different from linux
  REQUIRE( lv->sort_column("Date"));
  
  lv->set_item_image(0, wxART_WARNING);
  lv->items_update();
  
  wex::listview* lv2 = new wex::listview(wex::listview_data().type(wex::listview_data::FILE));
  add_pane(frame(), lv2);
  
  REQUIRE( lv2->data().image() == wex::listview_data::IMAGE_FILE_ICON);
  REQUIRE(!lv2->data().type_description().empty());
  
  REQUIRE( lv2->item_from_text("test.h\ntest.h"));
  REQUIRE(!lv2->item_to_text(0).empty());
  REQUIRE(!lv2->item_to_text(-1).empty());
  
  for (auto id : std::vector<int> {0}) 
  {
    wxListEvent* event = new wxListEvent(wxEVT_LIST_ITEM_ACTIVATED);
    event->m_itemIndex = id; // for wxWidgets 3.0 compatibility
    wxQueueEvent(lv2, event);
  }
  
  for (auto id : std::vector<int> {
    wex::ID_EDIT_SELECT_INVERT, wex::ID_EDIT_SELECT_NONE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(lv2, event);
  }
}
