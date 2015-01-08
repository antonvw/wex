////////////////////////////////////////////////////////////////////////////////
// Name:      test-textfile.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/frd.h>
#include <wx/extension/report/textfile.h>
#include "test.h"

void wxExGuiReportTestFixture::testTextFileWithListView()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();

  wxExListViewFileName* report = new wxExListViewFileName(
    frame, 
    wxExListViewFileName::LIST_FIND);
    
  wxExFindReplaceData::Get()->SetFindString("xx");
  
  CPPUNIT_ASSERT(wxExTextFileWithListView::SetupTool(tool, frame, report));
  
  wxExTextFileWithListView textFile(GetTestFile(), tool);
  CPPUNIT_ASSERT( textFile.RunTool());
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!textFile.IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT( textFile.RunTool()); // do the same test
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!textFile.IsOpened()); // file should be closed after running tool

  wxExTextFileWithListView textFile2(GetTestFile(), tool);
  CPPUNIT_ASSERT( textFile2.RunTool());
  CPPUNIT_ASSERT(!textFile2.GetStatistics().GetElements().GetItems().empty());
  
  wxExTool tool3(ID_TOOL_REPORT_KEYWORD);
  CPPUNIT_ASSERT(wxExTextFileWithListView::SetupTool(tool3, frame));
  wxExTextFileWithListView textFile3(GetTestFile(), tool3);
  CPPUNIT_ASSERT( textFile3.RunTool());
  CPPUNIT_ASSERT(!textFile3.GetStatistics().GetElements().GetItems().empty());
}
