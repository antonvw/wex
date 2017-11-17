////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
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
#include <wx/extension/stream.h>
#include <wx/extension/frd.h>
#include "test.h"

TEST_CASE("wxExStreamStatistics")
{
  wxExStreamStatistics ss;
  
  REQUIRE(ss.Get().empty());
  REQUIRE(ss.Get("xx") == 0);

  wxExStreamStatistics ss2;
  REQUIRE(ss2.Get().empty());

  ss += ss2;
  
  REQUIRE(ss.Get().empty());
}

TEST_CASE("wxExStream")
{
  SUBCASE("Test find")
  {
    wxExStream s(GetTestPath("test.h"), ID_TOOL_REPORT_FIND);
    
    REQUIRE( s.GetFileName() == GetTestPath("test.h"));
    REQUIRE( s.GetTool().GetId() == ID_TOOL_REPORT_FIND);
    
    wxExFindReplaceData::Get()->SetFindString("test");
    wxExFindReplaceData::Get()->SetMatchCase(true);
    wxExFindReplaceData::Get()->SetMatchWord(true);
    wxExFindReplaceData::Get()->SetUseRegEx(false);
    
    const auto start = std::chrono::system_clock::now();
    REQUIRE( s.RunTool());
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    
    REQUIRE(milli.count() < 100);
    REQUIRE(!s.GetStatistics().GetElements().GetItems().empty());
    REQUIRE( s.GetStatistics().Get("Actions Completed") == 193);
  }
  
  SUBCASE("Test replace")
  {
    wxExStream s(GetTestPath("test.h"), ID_TOOL_REPLACE);
    
    wxExFindReplaceData::Get()->SetReplaceString("test");
    
    const auto start = std::chrono::system_clock::now();
    REQUIRE( s.RunTool());
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    
    REQUIRE(milli.count() < 100);
    REQUIRE(!s.GetStatistics().GetElements().GetItems().empty());
    REQUIRE( s.GetStatistics().Get("Actions Completed") == 194);
  }
}
