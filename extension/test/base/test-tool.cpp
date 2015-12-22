////////////////////////////////////////////////////////////////////////////////
// Name:      test-tool.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/tool.h>
#include <wx/extension/statistics.h>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExTool" ) 
{
  const int id = 1000;
  
  wxExTool tool(id);
  tool.AddInfo(id, "this is ok");
  
  wxExStatistics<int> stat;
  
  REQUIRE( tool.GetId() == id);
  REQUIRE(!tool.GetToolInfo().empty());
  REQUIRE( tool.Info(&stat) == "this is ok 0 file(s)");
  REQUIRE(!tool.IsFindType());
  REQUIRE(!tool.IsReportType());
  
  REQUIRE(!wxExTool(ID_TOOL_REPORT_FIND).Info().empty());
  
  REQUIRE( wxExTool(ID_TOOL_REPORT_FIND).IsFindType());
  REQUIRE( wxExTool(ID_TOOL_REPORT_REPLACE).IsFindType());
}
