////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/stream.h>
#include <wex/ui/frd.h>

#include "test.h"

TEST_CASE("wex::stream")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  auto* lv = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::FIND));

  del_frame()->pane_add(lv);

  wex::find_replace_data::get()->set_find_string("method");

  wex::stream textFile(
    wex::find_replace_data::get(),
    wex::test::get_path("test.h"),
    tool,
    lv);

  REQUIRE(textFile.run_tool());
  REQUIRE(textFile.get_tool().id() == tool.id());
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  REQUIRE(textFile.run_tool()); // do the same test
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  wex::stream textFile2(
    wex::find_replace_data::get(),
    wex::test::get_path("test.h"),
    tool,
    lv);

  REQUIRE(textFile2.run_tool());
  REQUIRE(!textFile2.get_statistics().get_elements().get_items().empty());
}
