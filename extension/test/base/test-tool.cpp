////////////////////////////////////////////////////////////////////////////////
// Name:      test-tool.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/tool.h>
#include "test.h"

void TestFixture::testTool()
{
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_FIND).IsFindType());
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_REPLACE).IsFindType());
}
