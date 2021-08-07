////////////////////////////////////////////////////////////////////////////////
// Name:      test-listitem.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/listitem.h>

#include "test.h"

TEST_CASE("wex::listitem")
{
  auto* lv =
    new wex::listview(wex::data::listview().type(wex::data::listview::FILE));
  frame()->pane_add(lv);

  const auto start = std::chrono::system_clock::now();

  const int max = 1; // 250;
  for (int j = 0; j < max; j++)
  {
    wex::listitem item1(lv, wex::path("./test.h"));
    item1.insert();
    wex::listitem item2(lv, wex::path("./test-special.h"));
    item2.insert();
  }

  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - start);

  REQUIRE(milli.count() < 15000);

  const auto sort_start = std::chrono::system_clock::now();

  // The File Name column must be translated, otherwise test fails.
  lv->sort_column(_("File Name").ToStdString(), wex::SORT_ASCENDING);

  const auto sort_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - sort_start);

  REQUIRE(sort_milli.count() < 10000);

  REQUIRE(
    lv->get_item_text(0, _("File Name").ToStdString()).find("test-special.h") !=
    std::string::npos);

  wex::listitem item(lv, wex::path("./test.h"));
  item.insert();
  REQUIRE(item.path().filename() == "test.h");
  REQUIRE(item.file_spec().empty());
  REQUIRE(
    wex::listitem(lv, wex::path("./test.h"), "*.txt").file_spec() == "*.txt");
  REQUIRE(item.get_listview() == lv);
  REQUIRE(!item.is_readonly());

  item.set_item("xx", "yy");
  item.update();
  item.erase();
}
