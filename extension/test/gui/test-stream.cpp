////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/buffer.h>
#include <wex/stream.h>
#include <wex/frd.h>
#include "test.h"

TEST_CASE("wex::stream_statistics")
{
  wex::stream_statistics ss;
  
  REQUIRE(ss.Get().empty());
  REQUIRE(ss.Get("xx") == 0);

  wex::stream_statistics ss2;
  REQUIRE(ss2.Get().empty());

  ss += ss2;
  
  REQUIRE(ss.Get().empty());
}

TEST_CASE("wex::stream")
{
  SUBCASE("Test find")
  {
    wex::stream s(GetTestPath("test.h"), wex::ID_TOOL_REPORT_FIND);
    
    REQUIRE( s.GetFileName() == GetTestPath("test.h"));
    REQUIRE( s.GetTool().GetId() == wex::ID_TOOL_REPORT_FIND);
    
    wex::find_replace_data::Get()->SetFindString("test");
    wex::find_replace_data::Get()->SetMatchCase(true);
    wex::find_replace_data::Get()->SetMatchWord(true);
    wex::find_replace_data::Get()->SetUseRegEx(false);
    
    const auto start = std::chrono::system_clock::now();
    REQUIRE( s.RunTool());
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    
    REQUIRE(milli.count() < 100);
    REQUIRE(!s.GetStatistics().GetElements().GetItems().empty());
    REQUIRE( s.GetStatistics().Get("Actions Completed") == 193);
  }
  
  SUBCASE("Test replace")
  {
    wex::stream s(GetTestPath("test.h"), wex::ID_TOOL_REPLACE);
    
    wex::find_replace_data::Get()->SetReplaceString("test");
    
    const auto start = std::chrono::system_clock::now();
    REQUIRE( s.RunTool());
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    
    REQUIRE(milli.count() < 100);
    REQUIRE(!s.GetStatistics().GetElements().GetItems().empty());
    REQUIRE( s.GetStatistics().Get("Actions Completed") == 194);
  }
}
