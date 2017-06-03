////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h> // for wxArt
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExListView")
{
  wxExListView* listView = new wxExListView();
  AddPane(GetFrame(), listView);
  
  wxExListView::ConfigDialog(wxExWindowData().Button(wxAPPLY | wxCANCEL));
  
  REQUIRE(listView->GetData().Image() == IMAGE_ART);
  
  listView->ConfigGet();
  listView->SetSingleStyle(wxLC_REPORT);
  
  wxExColumn intcol(wxExColumn("Int", wxExColumn::COL_INT));
  REQUIRE(!intcol.GetIsSortedAscending());
  intcol.SetIsSortedAscending(SORT_ASCENDING);
  REQUIRE( intcol.GetIsSortedAscending());
  
  REQUIRE(listView->AppendColumn(intcol) == 0);
  REQUIRE(listView->AppendColumn(wxExColumn("Date", wxExColumn::COL_DATE)) == 1);
  REQUIRE(listView->AppendColumn(wxExColumn("Float", wxExColumn::COL_FLOAT)) == 2);
  REQUIRE(listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING)) == 3);

  REQUIRE(listView->FindColumn("Int") == 0);
  REQUIRE(listView->FindColumn("Date") == 1);
  REQUIRE(listView->FindColumn("Float") == 2);
  REQUIRE(listView->FindColumn("String") == 3);
  
  listView->InsertItem(0, "test");
  
  REQUIRE(listView->FindNext("test"));
  
  REQUIRE(listView->ItemFromText("a new item"));
  REQUIRE(listView->FindNext("a new item"));
  
  REQUIRE( listView->ItemToText(0).find("test") != std::string::npos);
  REQUIRE(!listView->ItemToText(-1).empty());
  
  //listView->Print(); // waits for input
  //listView->PrintPreview();

  // Delete all items, to test sorting later on.  
  listView->DeleteAllItems();
  
  listView->ItemsUpdate();
  
  for (int i = 0; i < 10; i++)
  {
    listView->InsertItem(i, std::to_string(i));
    listView->SetItem(i, 1, wxDateTime::Now().FormatISOCombined(' '));
    listView->SetItem(i, 2, wxString::Format("%f", (float)i / 2.0));
    listView->SetItem(i, 3, wxString::Format("hello %d", i));
  }
  
  // Test sorting.
  REQUIRE(!listView->SortColumn("xxx"));
  REQUIRE( listView->SortColumn("Int", SORT_ASCENDING));
  // TODO: OSX sort and menu events not ok
#ifndef __WXOSX__
  REQUIRE( listView->GetItemText(0, "Int") == "0");
  REQUIRE( listView->GetItemText(1, "Int") == "1");
  REQUIRE( listView->SortColumn("Int", SORT_DESCENDING));
  REQUIRE( listView->GetItemText(0, "Int") == "9");
  REQUIRE( listView->GetItemText(1, "Int") == "8");
#endif

  REQUIRE( listView->SortColumn("Date"));
  REQUIRE( listView->SortColumn("Float"));
  REQUIRE( listView->SortColumn("String"));
  
  REQUIRE( listView->GetSortedColumnNo() == 3);
  listView->SortColumnReset();
  REQUIRE( listView->GetSortedColumnNo() == -1);
  
  listView->SetItem(0, 1, "incorrect date");
  REQUIRE(!listView->SortColumn("Date"));
  
  listView->SetItemImage(0, wxART_WARNING);
  listView->ItemsUpdate();
  
  wxExListView* listView2 = new wxExListView(wxExListViewData().Type(LIST_FILE));
  AddPane(GetFrame(), listView2);
  
  REQUIRE( listView2->GetData().Image() == IMAGE_FILE_ICON);
  REQUIRE(!listView2->GetData().TypeDescription().empty());
  
  REQUIRE( listView2->ItemFromText("test.h\ntest.h"));
  REQUIRE(!listView2->ItemToText(0).empty());
  REQUIRE(!listView2->ItemToText(-1).empty());
  
  for (auto id : std::vector<int> {0}) 
  {
    wxListEvent* event = new wxListEvent(wxEVT_LIST_ITEM_ACTIVATED);
    event->SetIndex(id);
    wxQueueEvent(listView2, event);
  }
  
  for (auto id : std::vector<int> {
    ID_EDIT_SELECT_INVERT, ID_EDIT_SELECT_NONE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView2, event);
  }

  REQUIRE(wxExUIAction(listView));
}
