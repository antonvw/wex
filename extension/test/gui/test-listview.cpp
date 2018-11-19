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
  wex::listview* listView = new wex::listview();
  AddPane(frame(), listView);
  
  wex::listview::config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL));
  
  REQUIRE(listView->data().image() == wex::listview_data::IMAGE_ART);
  
  listView->config_get();
  
  wex::column intcol(wex::column("Int", wex::column::INT));
  REQUIRE(!intcol.is_sorted_ascending());
  intcol.set_is_sorted_ascending(wex::SORT_ASCENDING);
  REQUIRE( intcol.is_sorted_ascending());
  
  REQUIRE( listView->append_columns({{intcol}}));
  REQUIRE( listView->append_columns({
    {"Date", wex::column::DATE},
    {"Float", wex::column::FLOAT},
    {"String", wex::column::STRING}}));

  REQUIRE(listView->find_column("Int") == 0);
  REQUIRE(listView->find_column("Date") == 1);
  REQUIRE(listView->find_column("Float") == 2);
  REQUIRE(listView->find_column("String") == 3);
  
  REQUIRE( listView->insert_item({"95"}));
  REQUIRE(!listView->insert_item({"test"}));
  REQUIRE(!listView->insert_item({"1", "2", "3", "4", "5"}));
  
  REQUIRE( listView->find_next("95"));
  REQUIRE(!listView->find_next("test"));
  
  REQUIRE(!listView->item_from_text("a new item"));
  REQUIRE(!listView->find_next("another new item"));
  REQUIRE( listView->item_from_text("999"));
  
  REQUIRE( listView->item_to_text(0).find("95") != std::string::npos);
  REQUIRE(!listView->item_to_text(-1).empty());
  
  //listView->print(); // waits for input
  //listView->print_preview();

  // Delete all items, to test sorting later on.  
  listView->DeleteAllItems();
  
  listView->items_update();
  
  for (int i = 0; i < 10; i++)
  {
    REQUIRE( listView->insert_item({
      std::to_string(i),
      GetTestPath("test.h").stat().get_modification_time(),
      std::to_string((float)i / 2.0),
      "hello " + std::to_string(i)}));
  }
  
  // Test sorting.
  REQUIRE(!listView->sort_column("xxx"));
  REQUIRE( listView->sort_column("Int", wex::SORT_ASCENDING));
  REQUIRE( listView->get_item_text(0, "Int") == "0");
  REQUIRE( listView->get_item_text(1, "Int") == "1");
  REQUIRE( listView->sort_column("Int", wex::SORT_DESCENDING));
  REQUIRE( listView->get_item_text(0, "Int") == "9");
  REQUIRE( listView->get_item_text(1, "Int") == "8");
  REQUIRE( listView->sort_column("Date"));
  REQUIRE( listView->sort_column("Float"));
  REQUIRE( listView->sort_column("String"));
  
  REQUIRE( listView->sorted_column_no() == 3);
  listView->sort_column_reset();
  REQUIRE( listView->sorted_column_no() == -1);
  
  listView->SetItem(0, 1, "incorrect date"); // return value on osx different from linux
  REQUIRE( listView->sort_column("Date"));
  
  listView->set_item_image(0, wxART_WARNING);
  listView->items_update();
  
  wex::listview* listView2 = new wex::listview(wex::listview_data().type(wex::listview_data::FILE));
  AddPane(frame(), listView2);
  
  REQUIRE( listView2->data().image() == wex::listview_data::IMAGE_FILE_ICON);
  REQUIRE(!listView2->data().type_description().empty());
  
  REQUIRE( listView2->item_from_text("test.h\ntest.h"));
  REQUIRE(!listView2->item_to_text(0).empty());
  REQUIRE(!listView2->item_to_text(-1).empty());
  
  for (auto id : std::vector<int> {0}) 
  {
    wxListEvent* event = new wxListEvent(wxEVT_LIST_ITEM_ACTIVATED);
    event->m_itemIndex = id; // for wxWidgets 3.0 compatibility
    wxQueueEvent(listView2, event);
  }
  
  for (auto id : std::vector<int> {
    wex::ID_EDIT_SELECT_INVERT, wex::ID_EDIT_SELECT_NONE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView2, event);
  }
}
