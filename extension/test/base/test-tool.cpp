////////////////////////////////////////////////////////////////////////////////
// Name:      test-tool.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/tool.h>
#include <wx/extension/statistics.h>
#include "test.h"

void TestFixture::testTool()
{
  const int id = 1000;
  
  wxExTool tool(id);
  tool.AddInfo(id, "this is ok");
  
  wxExStatistics<int> stat;
  
  CPPUNIT_ASSERT( tool.GetId() == id);
  CPPUNIT_ASSERT(!tool.GetToolInfo().empty());
  CPPUNIT_ASSERT( tool.Info(&stat) == "this is ok 0 file(s)");
  CPPUNIT_ASSERT(!tool.IsFindType());
  CPPUNIT_ASSERT(!tool.IsReportType());
  
  CPPUNIT_ASSERT(!wxExTool(ID_TOOL_REPORT_FIND).Info().empty());
  
  CPPUNIT_ASSERT( wxExTool(ID_TOOL_REPORT_FIND).IsFindType());
  CPPUNIT_ASSERT( wxExTool(ID_TOOL_REPORT_REPLACE).IsFindType());
}
