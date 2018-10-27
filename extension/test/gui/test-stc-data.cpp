////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/stc-data.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::stc_data")
{
  wex::stc* stc = GetSTC();

  SUBCASE("Constructor")
  {
    REQUIRE( wex::stc_data().Control().Line() == 0);
    REQUIRE( wex::stc_data().Control(
      wex::control_data().Col(3)).Control().Col() == 3);
    REQUIRE( wex::stc_data(
      wex::control_data().Col(3)).Control().Col() == 3);
    REQUIRE( wex::stc_data(
      wex::window_data().Name("XX")).Window().Name() == "XX");
    REQUIRE( wex::stc_data().Flags(
      wex::stc_data::WIN_READ_ONLY).Flags() == wex::stc_data::WIN_READ_ONLY);
    REQUIRE( wex::stc_data().Flags(
      wex::stc_data::WIN_READ_ONLY).Flags(wex::stc_data::WIN_HEX, wex::control_data::OR).
      Flags() != wex::stc_data::WIN_READ_ONLY);
    REQUIRE((wex::stc_data().Menu() & wex::stc_data::MENU_VCS));
    REQUIRE((wex::stc_data().CTagsFileName() == "tags"));
  }
  
  SUBCASE("Inject")
  {
    stc->SetText("line 1\nline 2\nline 3\n");
    REQUIRE( wex::stc_data(stc).Control(
      wex::control_data().Line(1).Col(5)).Inject());
    REQUIRE( stc->GetCurrentLine() == 0);
    REQUIRE( stc->GetCurrentPos() == 4);
    REQUIRE( wex::stc_data(stc, wex::control_data().Line(1).Col(5)).Inject());
    REQUIRE(!wex::stc_data().Control(
      wex::control_data().Line(1).Col(5)).Inject());
  }
}
