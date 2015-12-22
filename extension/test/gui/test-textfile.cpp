////////////////////////////////////////////////////////////////////////////////
// Name:      test-textfile.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/buffer.h>
#include <wx/extension/textfile.h>
#include <wx/extension/frd.h>
#include "test.h"

TEST_CASE("wxExFileStatistics")
{
  wxExFileStatistics fileStatistics;
  
  REQUIRE(fileStatistics.Get().empty());
  REQUIRE(fileStatistics.Get("xx") == 0);

  wxExFileStatistics fileStatistics2;
  REQUIRE(fileStatistics2.Get().empty());

  fileStatistics += fileStatistics2;
  
  REQUIRE(fileStatistics.Get().empty());
}

TEST_CASE("wxExTextFile")
{
  // Test find.
  wxExTextFile textFile(GetTestFile(), ID_TOOL_REPORT_FIND);
  
  REQUIRE( textFile.GetFileName() == GetTestFile());
  REQUIRE( textFile.GetTool().GetId() == ID_TOOL_REPORT_FIND);
  
  wxExFindReplaceData::Get()->SetFindString("test");
  wxExFindReplaceData::Get()->SetMatchCase(true);
  wxExFindReplaceData::Get()->SetMatchWord(true);
  wxExFindReplaceData::Get()->SetUseRegEx(false);
  
  wxStopWatch sw;
  sw.Start();
  
  REQUIRE( textFile.RunTool());
  
  const long elapsed = sw.Time();
  
  REQUIRE(elapsed < 20);
  
  INFO(wxString::Format(
    "wxExTextFile::matching %d items in %ld ms", 
    textFile.GetStatistics().Get(_("Actions Completed")), elapsed).ToStdString());
    
  REQUIRE(!textFile.GetStatistics().GetElements().GetItems().empty());
  REQUIRE( textFile.GetStatistics().Get(_("Actions Completed")) == 193);
  
  // Test replace.
  wxExTextFile textFile2(GetTestFile(), ID_TOOL_REPORT_REPLACE);
  
  wxExFindReplaceData::Get()->SetReplaceString("test");
  
  wxStopWatch sw2;
  sw2.Start();
  REQUIRE( textFile2.RunTool());
  const long elapsed2 = sw2.Time();
  
  REQUIRE(elapsed2 < 100);
  
  INFO(wxString::Format(
    "wxExTextFile::replacing %d items in %ld ms", 
    textFile2.GetStatistics().Get(_("Actions Completed")), elapsed2).ToStdString());
    
  REQUIRE(!textFile2.GetStatistics().GetElements().GetItems().empty());
  REQUIRE( textFile2.GetStatistics().Get(_("Actions Completed")) == 194);
}
