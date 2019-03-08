////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/frd.h>
#include <wex/report/stream.h>
#include "test.h"

TEST_CASE("wex::report::stream")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  wex::listview* report = new wex::listview(wex::listview_data().type(wex::listview_data::FIND));
    
  wex::test::add_pane(frame(), report);

  wex::find_replace_data::get()->set_find_string("xx");
  
  REQUIRE(wex::report::stream::setup_tool(tool, frame(), report));
  
  wex::report::stream textFile(wex::test::get_path("test.h"), tool);
  
  REQUIRE( textFile.run_tool());
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  REQUIRE( textFile.run_tool()); // do the same test
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  wex::report::stream textFile2(wex::test::get_path("test.h"), tool);
  REQUIRE( textFile2.run_tool());
  REQUIRE(!textFile2.get_statistics().get_elements().get_items().empty());
  
  wex::tool tool3(wex::ID_TOOL_REPORT_KEYWORD);
  REQUIRE(wex::report::stream::setup_tool(tool3, frame()));
  wex::report::stream textFile3(wex::test::get_path("test.h"), tool3);
  REQUIRE( textFile3.run_tool());
  REQUIRE(!textFile3.get_statistics().get_elements().get_items().empty());
}
