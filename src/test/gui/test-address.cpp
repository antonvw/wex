////////////////////////////////////////////////////////////////////////////////
// Name:      test-address.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/address.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/vi-macros.h>
#include "test.h"

TEST_CASE("wex::address")
{
  wex::stc* stc = get_stc();
  stc->set_text("hello0\nhello1\nhello2\nhello3\nhello4\nhello5");
  
  const int lines = stc->GetLineCount();
  wex::ex* ex = new wex::ex(stc);
  wex::stc_data(stc).control(wex::control_data().line(1)).inject();
  ex->marker_add('a'); // put marker a on line
  wex::stc_data(stc).control(wex::control_data().line(2)).inject();
  ex->marker_add('b'); // put marker b on line
  
  REQUIRE( wex::address(ex).get_line() == 0);
  
  for (const auto& it : std::vector< std::pair<std::string, int>> {
    {"30", lines},
    {"40", lines},
    {"-40", 1},
    {"3-3", 0},
    {"3-1", 2},
    {".", 2},
    {".+1", 3},
    {"$", lines},
    {"$-2", lines - 2},
    {"x", 0},
    {"'x", 0},
    {"1,3s/x/y", 0},
    {"/2/", 3},
    {"?2?", 3},
    {"'a", 1},
    {"'b", 2},
    {"'b+10", lines},
    {"10+'b", lines},
    {"'a+'b", 3},
    {"'b+'a", 3},
    {"'b-'a", 1}})
  {
    REQUIRE( wex::address(ex, it.first).get_line() == it.second);
  }

  wex::address address3(ex, "5");
  
  // Test adjust_window.
  REQUIRE( address3.adjust_window(""));
  REQUIRE( address3.adjust_window("-"));
  REQUIRE( address3.adjust_window("+"));
  REQUIRE( address3.adjust_window("^"));
  REQUIRE( address3.adjust_window("="));
  REQUIRE( address3.adjust_window("."));
  REQUIRE(!address3.adjust_window("xxx"));
  
  // Test append.
  REQUIRE( address3.append("appended text"));
  REQUIRE( stc->GetText().Contains("appended text"));
  
  // Test Flags.
  REQUIRE( address3.flags_supported(""));
  REQUIRE( address3.flags_supported("#"));
  REQUIRE(!address3.flags_supported("x"));

  // Test get, get_line.
  REQUIRE( wex::address(ex).get_line() == 0);
  REQUIRE( wex::address(ex, "-1").get_line() == 1);
  REQUIRE( wex::address(ex, "-1").get() == "-1");
  REQUIRE( wex::address(ex, "1").get_line() == 1);
  REQUIRE( wex::address(ex, "1").get() == "1");
  REQUIRE( wex::address(ex, "100").get_line() == lines);
  
  wex::address address2(ex, "'a");
  REQUIRE( address2.get_line() == 1);
  address2.marker_delete();
  REQUIRE( address2.get_line() == 0);
  
  // Test Insert.
  REQUIRE( address3.insert("inserted text"));
  REQUIRE( stc->GetText().Contains("inserted text"));
  
  // Test marker_add.
  REQUIRE( address3.marker_add('x'));
  
  // Test marker_delete.
  REQUIRE(!address3.marker_delete());
  REQUIRE( wex::address(ex, "'x").marker_delete());
  
  // Test put.
  ex->get_macros().set_register('z', "zzzzz");
  REQUIRE( address3.put('z'));
  REQUIRE( stc->GetText().Contains("zzzz"));
  
  // Test Read.
  REQUIRE(!address3.read("XXXXX"));
  REQUIRE( address3.read(wex::test::get_path("test.bin").string()));
#ifdef __UNIX__
  REQUIRE( address3.read("!ls"));
#endif
  
  // Test write_line_number.
  REQUIRE( address3.write_line_number());
  
  stc->clear();
}
