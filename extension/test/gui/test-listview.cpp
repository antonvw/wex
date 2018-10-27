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
  AddPane(GetFrame(), listView);
  
  wex::listview::ConfigDialog(wex::window_data().Button(wxAPPLY | wxCANCEL));
  
  REQUIRE(listView->GetData().Image() == wex::listview_data::IMAGE_ART);
  
  listView->ConfigGet();
  
  wex::column intcol(wex::column("Int", wex::column::INT));
  REQUIRE(!intcol.GetIsSortedAscending());
  intcol.SetIsSortedAscending(wex::SORT_ASCENDING);
  REQUIRE( intcol.GetIsSortedAscending());
  
  REQUIRE( listView->AppendColumns({{intcol}}));
  REQUIRE( listView->AppendColumns({
    {"Date", wex::column::DATE},
    {"Float", wex::column::FLOAT},
    {"String", wex::column::STRING}}));

  REQUIRE(listView->FindColumn("Int") == 0);
  REQUIRE(listView->FindColumn("Date") == 1);
  REQUIRE(listView->FindColumn("Float") == 2);
  REQUIRE(listView->FindColumn("String") == 3);
  
  REQUIRE( listView->InsertItem({"95"}));
  REQUIRE(!listView->InsertItem({"test"}));
  REQUIRE(!listView->InsertItem({"1", "2", "3", "4", "5"}));
  
  REQUIRE( listView->FindNext("95"));
  REQUIRE(!listView->FindNext("test"));
  
  REQUIRE(!listView->ItemFromText("a new item"));
  REQUIRE(!listView->FindNext("another new item"));
  REQUIRE( listView->ItemFromText("999"));
  
  REQUIRE( listView->ItemToText(0).find("95") != std::string::npos);
  REQUIRE(!listView->ItemToText(-1).empty());
  
  //listView->Print(); // waits for input
  //listView->PrintPreview();

  // Delete all items, to test sorting later on.  
  listView->DeleteAllItems();
  
  listView->ItemsUpdate();
  
  for (int i = 0; i < 10; i++)
  {
    REQUIRE( listView->InsertItem({
      std::to_string(i),
      GetTestPath("test.h").GetStat().get_modification_time(),
      std::to_string((float)i / 2.0),
      "hello " + std::to_string(i)}));
  }
  
  // Test sorting.
  REQUIRE(!listView->SortColumn("xxx"));
  REQUIRE( listView->SortColumn("Int", wex::SORT_ASCENDING));
  REQUIRE( listView->GetItemText(0, "Int") == "0");
  REQUIRE( listView->GetItemText(1, "Int") == "1");
  REQUIRE( listView->SortColumn("Int", wex::SORT_DESCENDING));
  REQUIRE( listView->GetItemText(0, "Int") == "9");
  REQUIRE( listView->GetItemText(1, "Int") == "8");
  REQUIRE( listView->SortColumn("Date"));
  REQUIRE( listView->SortColumn("Float"));
  REQUIRE( listView->SortColumn("String"));
  
  REQUIRE( listView->GetSortedColumnNo() == 3);
  listView->SortColumnReset();
  REQUIRE( listView->GetSortedColumnNo() == -1);
  
  listView->SetItem(0, 1, "incorrect date"); // return value on osx different from linux
  REQUIRE( listView->SortColumn("Date"));
  
  listView->SetItemImage(0, wxART_WARNING);
  listView->ItemsUpdate();
  
  wex::listview* listView2 = new wex::listview(wex::listview_data().Type(wex::listview_data::FILE));
  AddPane(GetFrame(), listView2);
  
  REQUIRE( listView2->GetData().Image() == wex::listview_data::IMAGE_FILE_ICON);
  REQUIRE(!listView2->GetData().TypeDescription().empty());
  
  REQUIRE( listView2->ItemFromText("test.h\ntest.h"));
  REQUIRE(!listView2->ItemToText(0).empty());
  REQUIRE(!listView2->ItemToText(-1).empty());
  
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
