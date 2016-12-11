////////////////////////////////////////////////////////////////////////////////
// Name:      test-textfile.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
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
  
  const auto start = std::chrono::system_clock::now();
  
  REQUIRE( textFile.RunTool());
  
  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  REQUIRE(milli.count() < 100);
  
  INFO(wxString::Format(
    "wxExTextFile::matching %d items in %d ms", 
    textFile.GetStatistics().Get(_("Actions Completed")), (int)milli.count()).ToStdString());
    
  REQUIRE(!textFile.GetStatistics().GetElements().GetItems().empty());
  REQUIRE( textFile.GetStatistics().Get(_("Actions Completed")) == 193);
  
  // Test replace.
  wxExTextFile textFile2(GetTestFile(), ID_TOOL_REPORT_REPLACE);
  
  wxExFindReplaceData::Get()->SetReplaceString("test");
  
  const auto start2 = std::chrono::system_clock::now();
  REQUIRE( textFile2.RunTool());
  const auto milli2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start2);
  
  REQUIRE(milli2.count() < 100);
  
  INFO(wxString::Format(
    "wxExTextFile::replacing %d items in %d ms", 
    textFile2.GetStatistics().Get(_("Actions Completed")), (int)milli2.count()).ToStdString());
    
  REQUIRE(!textFile2.GetStatistics().GetElements().GetItems().empty());
  REQUIRE( textFile2.GetStatistics().Get(_("Actions Completed")) == 194);
}
