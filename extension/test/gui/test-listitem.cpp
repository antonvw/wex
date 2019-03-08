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
#include <wex/listitem.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::listitem")
{
  wex::listview* listView = new wex::listview(wex::listview_data().type(wex::listview_data::FILE));
  wex::test::add_pane(frame(), listView);
  
  const auto start = std::chrono::system_clock::now();

  const int max = 1; // 250;
  for (int j = 0; j < max; j++)
  {
    wex::listitem item1(listView, wex::path("./test.h"));
    item1.insert();
    wex::listitem item2(listView, wex::path("./test-special.h"));
    item2.insert();
  }

  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);

  REQUIRE(milli.count() < 15000);
  
  const auto sort_start = std::chrono::system_clock::now();
  
  // The File Name column must be translated, otherwise test fails.
  listView->sort_column(_("File Name").ToStdString(), wex::SORT_ASCENDING);
  
  const auto sort_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - sort_start);
  
  REQUIRE(sort_milli.count() < 10000);
  
  REQUIRE(listView->get_item_text(0, _("File Name").ToStdString()).find("test-special.h") != std::string::npos);
  
  wex::listitem item(listView, wex::path("./test.h"));
  item.insert();
  REQUIRE( item.get_filename().fullname() == "test.h");
  REQUIRE( item.file_spec().empty());
  REQUIRE( wex::listitem(listView, 
    wex::path("./test.h"), "*.txt").file_spec() == "*.txt");
  REQUIRE( item.get_listview() == listView);
  REQUIRE(!item.is_readOnly());
  
  item.set_item("xx", "yy");
  item.update();
  item.erase();
}
