////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/stream.h>
#include <wex/frd.h>

#include "test.h"

TEST_CASE("wex::del::stream")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  auto* report = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::FIND));

  del_frame()->pane_add(report);

  wex::find_replace_data::get()->set_find_string("xx");

  REQUIRE(wex::del::stream::setup_tool(tool, del_frame(), report));

  wex::del::stream textFile(wex::test::get_path("test.h"), tool);

  REQUIRE(textFile.run_tool());
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  REQUIRE(textFile.run_tool()); // do the same test
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  wex::del::stream textFile2(wex::test::get_path("test.h"), tool);
  REQUIRE(textFile2.run_tool());
  REQUIRE(!textFile2.get_statistics().get_elements().get_items().empty());

#ifndef __WXMSW__
  wex::tool tool3(wex::ID_TOOL_REPORT_KEYWORD);
  REQUIRE(wex::del::stream::setup_tool(tool3, del_frame()));
  wex::del::stream textFile3(wex::test::get_path("test.h"), tool3);
  REQUIRE(textFile3.run_tool());
  REQUIRE(!textFile3.get_statistics().get_elements().get_items().empty());
#endif
}
