////////////////////////////////////////////////////////////////////////////////
// Name:      test-textfile.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/frd.h>
#include <wx/extension/report/textfile.h>
#include "test.h"

TEST_CASE("wxExTextFileWithListView")
{
  wxExTool tool(ID_TOOL_REPORT_FIND);

  wxExListView* report = new wxExListView(
    GetFrame(), 
    wxExListView::LIST_FIND);
    
  AddPane(GetFrame(), report);

  wxExFindReplaceData::Get()->SetFindString("xx");
  
  REQUIRE(wxExTextFileWithListView::SetupTool(tool, GetFrame(), report));
  
  wxExTextFileWithListView textFile(GetTestFile(), tool);
  
  REQUIRE( textFile.RunTool());
  REQUIRE(!textFile.GetStatistics().GetElements().GetItems().empty());
  REQUIRE(!textFile.IsOpened()); // file should be closed after running tool

  REQUIRE( textFile.RunTool()); // do the same test
  REQUIRE(!textFile.GetStatistics().GetElements().GetItems().empty());
  REQUIRE(!textFile.IsOpened()); // file should be closed after running tool

  wxExTextFileWithListView textFile2(GetTestFile(), tool);
  REQUIRE( textFile2.RunTool());
  REQUIRE(!textFile2.GetStatistics().GetElements().GetItems().empty());
  
  wxExTool tool3(ID_TOOL_REPORT_KEYWORD);
  REQUIRE(wxExTextFileWithListView::SetupTool(tool3, GetFrame()));
  wxExTextFileWithListView textFile3(GetTestFile(), tool3);
  REQUIRE( textFile3.RunTool());
  REQUIRE(!textFile3.GetStatistics().GetElements().GetItems().empty());
}
