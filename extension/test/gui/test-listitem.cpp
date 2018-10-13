////////////////////////////////////////////////////////////////////////////////
// Name:      test-listitem.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/listitem.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wex::listitem")
{
  wex::listview* listView = new wex::listview(wex::listview_data().Type(wex::LISTVIEW_FILE));
  AddPane(GetFrame(), listView);
  
  const auto start = std::chrono::system_clock::now();

  const int max = 1; // 250;
  for (int j = 0; j < max; j++)
  {
    wex::listitem item1(listView, wex::path("./test.h"));
    item1.Insert();
    wex::listitem item2(listView, wex::path("./test-special.h"));
    item2.Insert();
  }

  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

  REQUIRE(milli.count() < 15000);
  
  const auto sort_start = std::chrono::system_clock::now();
  
  // The File Name column must be translated, otherwise test fails.
  listView->SortColumn(_("File Name").ToStdString(), wex::SORT_ASCENDING);
  
  const auto sort_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - sort_start);
  
  REQUIRE(sort_milli.count() < 10000);
  
  REQUIRE(listView->GetItemText(0, _("File Name").ToStdString()).find("test-special.h") != std::string::npos);
  
  wex::listitem item(listView, wex::path("./test.h"));
  item.Insert();
  REQUIRE( item.GetFileName().GetFullName() == "test.h");
  REQUIRE( item.GetFileSpec().empty());
  REQUIRE( wex::listitem(listView, 
    wex::path("./test.h"), "*.txt").GetFileSpec() == "*.txt");
  REQUIRE( item.GetListView() == listView);
  REQUIRE(!item.IsReadOnly());
  
  item.SetItem("xx", "yy");
  item.Update();
  item.Delete();
}
