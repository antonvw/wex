////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/frd.h>
#include <wex/report/stream.h>
#include "test.h"

TEST_CASE("wex::listview_stream")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  wex::listview* report = new wex::listview(wex::listview_data().type(wex::listview_data::FIND));
    
  AddPane(frame(), report);

  wex::find_replace_data::get()->set_find_string("xx");
  
  REQUIRE(wex::listview_stream::setup_tool(tool, frame(), report));
  
  wex::listview_stream textFile(GetTestPath("test.h"), tool);
  
  REQUIRE( textFile.run_tool());
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  REQUIRE( textFile.run_tool()); // do the same test
  REQUIRE(!textFile.get_statistics().get_elements().get_items().empty());

  wex::listview_stream textFile2(GetTestPath("test.h"), tool);
  REQUIRE( textFile2.run_tool());
  REQUIRE(!textFile2.get_statistics().get_elements().get_items().empty());
  
  wex::tool tool3(wex::ID_TOOL_REPORT_KEYWORD);
  REQUIRE(wex::listview_stream::setup_tool(tool3, frame()));
  wex::listview_stream textFile3(GetTestPath("test.h"), tool3);
  REQUIRE( textFile3.run_tool());
  REQUIRE(!textFile3.get_statistics().get_elements().get_items().empty());
}
