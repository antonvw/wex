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
  tool.add_info(id, "this is ok");
  
  wex::statistics<int> stat;
  
  REQUIRE( tool.id() == id);
  REQUIRE(!tool.get_tool_info().empty());
  REQUIRE( tool.info(&stat) == "this is ok 0 file(s)");
  REQUIRE(!tool.is_find_type());
  REQUIRE(!tool.is_report_type());
  
  REQUIRE(!wex::tool(wex::ID_TOOL_REPORT_FIND).info().empty());
  
  REQUIRE( wex::tool(wex::ID_TOOL_REPORT_FIND).is_find_type());
  REQUIRE( wex::tool(wex::ID_TOOL_REPLACE).is_find_type());
}
