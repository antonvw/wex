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
#include <wx/extension/address.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi-macros.h>
#include "test.h"

TEST_CASE("wex::address")
{
  wex::stc* stc = GetSTC();
  stc->SetText("hello0\nhello1\nhello2\nhello3\nhello4\nhello5");
  
  const int lines = stc->GetLineCount();
  wex::ex* ex = new wex::ex(stc);
  wex::stc_data(stc).Control(wex::control_data().Line(1)).Inject();
  ex->MarkerAdd('a'); // put marker a on line
  wex::stc_data(stc).Control(wex::control_data().Line(2)).Inject();
  ex->MarkerAdd('b'); // put marker b on line
  
  REQUIRE( wex::address(ex).GetLine() == 0);
  
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
    REQUIRE( wex::address(ex, it.first).GetLine() == it.second);
  }

  wex::address address3(ex, "5");
  
  // Test AdjustWindow.
  REQUIRE( address3.AdjustWindow(""));
  REQUIRE( address3.AdjustWindow("-"));
  REQUIRE( address3.AdjustWindow("+"));
  REQUIRE( address3.AdjustWindow("^"));
  REQUIRE( address3.AdjustWindow("="));
  REQUIRE( address3.AdjustWindow("."));
  REQUIRE(!address3.AdjustWindow("xxx"));
  
  // Test Append.
  REQUIRE( address3.Append("appended text"));
  REQUIRE( stc->GetText().Contains("appended text"));
  
  // Test Flags.
  REQUIRE( address3.Flags(""));
  REQUIRE( address3.Flags("#"));
  REQUIRE(!address3.Flags("x"));

  // Test Get, GetLine.
  REQUIRE( wex::address(ex).GetLine() == 0);
  REQUIRE( wex::address(ex, "-1").GetLine() == 1);
  REQUIRE( wex::address(ex, "-1").Get() == "-1");
  REQUIRE( wex::address(ex, "1").GetLine() == 1);
  REQUIRE( wex::address(ex, "1").Get() == "1");
  REQUIRE( wex::address(ex, "100").GetLine() == lines);
  
  wex::address address2(ex, "'a");
  REQUIRE( address2.GetLine() == 1);
  address2.MarkerDelete();
  REQUIRE( address2.GetLine() == 0);
  
  // Test Insert.
  REQUIRE( address3.Insert("inserted text"));
  REQUIRE( stc->GetText().Contains("inserted text"));
  
  // Test MarkerAdd.
  REQUIRE( address3.MarkerAdd('x'));
  
  // Test MarkerDelete.
  REQUIRE(!address3.MarkerDelete());
  REQUIRE( wex::address(ex, "'x").MarkerDelete());
  
  // Test Put.
  ex->GetMacros().SetRegister('z', "zzzzz");
  REQUIRE( address3.Put('z'));
  REQUIRE( stc->GetText().Contains("zzzz"));
  
  // Test Read.
  REQUIRE(!address3.Read("XXXXX"));
  REQUIRE( address3.Read(GetTestPath("test.bin").Path().string()));
#ifdef __UNIX__
  REQUIRE( address3.Read("!ls"));
#endif
  
  // Test WriteLineNumber.
  REQUIRE( address3.WriteLineNumber());
  
  stc->ClearDocument();
}
