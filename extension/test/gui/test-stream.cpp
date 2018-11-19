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
  
  REQUIRE(ss.get().empty());
  REQUIRE(ss.get("xx") == 0);

  wex::stream_statistics ss2;
  REQUIRE(ss2.get().empty());

  ss += ss2;
  
  REQUIRE(ss.get().empty());
}

TEST_CASE("wex::stream")
{
  SUBCASE("Test find")
  {
    wex::stream s(GetTestPath("test.h"), wex::ID_TOOL_REPORT_FIND);
    
    REQUIRE( s.get_filename() == GetTestPath("test.h"));
    REQUIRE( s.get_tool().id() == wex::ID_TOOL_REPORT_FIND);
    
    wex::find_replace_data::get()->set_find_string("test");
    wex::find_replace_data::get()->set_match_case(true);
    wex::find_replace_data::get()->set_match_word(true);
    wex::find_replace_data::get()->set_use_regex(false);
    
    const auto start = std::chrono::system_clock::now();
    REQUIRE( s.run_tool());
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    
    REQUIRE(milli.count() < 100);
    REQUIRE(!s.get_statistics().get_elements().get_items().empty());
    REQUIRE( s.get_statistics().get("Actions Completed") == 193);
  }
  
  SUBCASE("Test replace")
  {
    wex::stream s(GetTestPath("test.h"), wex::ID_TOOL_REPLACE);
    
    wex::find_replace_data::get()->set_replace_string("test");
    
    const auto start = std::chrono::system_clock::now();
    REQUIRE( s.run_tool());
    const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
    
    REQUIRE(milli.count() < 100);
    REQUIRE(!s.get_statistics().get_elements().get_items().empty());
    REQUIRE( s.get_statistics().get("Actions Completed") == 194);
  }
}
