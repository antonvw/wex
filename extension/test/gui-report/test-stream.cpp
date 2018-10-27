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

  wex::listview* report = new wex::listview(wex::listview_data().Type(wex::listview_data::FIND));
    
  AddPane(GetFrame(), report);

  wex::find_replace_data::Get()->SetFindString("xx");
  
  REQUIRE(wex::listview_stream::SetupTool(tool, GetFrame(), report));
  
  wex::listview_stream textFile(GetTestPath("test.h"), tool);
  
  REQUIRE( textFile.RunTool());
  REQUIRE(!textFile.GetStatistics().GetElements().GetItems().empty());

  REQUIRE( textFile.RunTool()); // do the same test
  REQUIRE(!textFile.GetStatistics().GetElements().GetItems().empty());

  wex::listview_stream textFile2(GetTestPath("test.h"), tool);
  REQUIRE( textFile2.RunTool());
  REQUIRE(!textFile2.GetStatistics().GetElements().GetItems().empty());
  
  wex::tool tool3(wex::ID_TOOL_REPORT_KEYWORD);
  REQUIRE(wex::listview_stream::SetupTool(tool3, GetFrame()));
  wex::listview_stream textFile3(GetTestPath("test.h"), tool3);
  REQUIRE( textFile3.RunTool());
  REQUIRE(!textFile3.GetStatistics().GetElements().GetItems().empty());
}
