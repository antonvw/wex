////////////////////////////////////////////////////////////////////////////////
// Name:      test-tool.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/tool.h>
#include <wex/statistics.h>
#include "../test.h"

TEST_CASE( "wex::tool" ) 
{
  const int id = 1000;
  
  wex::tool tool(id);
  tool.AddInfo(id, "this is ok");
  
  wex::statistics<int> stat;
  
  REQUIRE( tool.GetId() == id);
  REQUIRE(!tool.GetToolInfo().empty());
  REQUIRE( tool.Info(&stat) == "this is ok 0 file(s)");
  REQUIRE(!tool.IsFindType());
  REQUIRE(!tool.IsReportType());
  
  REQUIRE(!wex::tool(wex::ID_TOOL_REPORT_FIND).Info().empty());
  
  REQUIRE( wex::tool(wex::ID_TOOL_REPORT_FIND).IsFindType());
  REQUIRE( wex::tool(wex::ID_TOOL_REPLACE).IsFindType());
}
