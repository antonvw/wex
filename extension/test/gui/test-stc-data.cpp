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
#include <wx/extension/stc-data.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::stc_data")
{
  wex::stc* stc = GetSTC();

  SUBCASE("Constructor")
  {
    REQUIRE( wex::stc_data().Control().Line() == 0);
    REQUIRE( wex::stc_data().Control(wex::control_data().Col(3)).Control().Col() == 3);
    REQUIRE( wex::stc_data(wex::control_data().Col(3)).Control().Col() == 3);
    REQUIRE( wex::stc_data(wex::window_data().Name("XX")).Window().Name() == "XX");
    REQUIRE( wex::stc_data().Flags(wex::STC_WIN_READ_ONLY).Flags() == wex::STC_WIN_READ_ONLY);
    REQUIRE( wex::stc_data().Flags(wex::STC_WIN_READ_ONLY).Flags(wex::STC_WIN_HEX, wex::DATA_OR).
      Flags() != wex::STC_WIN_READ_ONLY);
    REQUIRE((wex::stc_data().Menu() & wex::STC_MENU_VCS));
    REQUIRE((wex::stc_data().CTagsFileName() == "tags"));
  }
  
  SUBCASE("Inject")
  {
    stc->SetText("line 1\nline 2\nline 3\n");
    REQUIRE( wex::stc_data(stc).Control(wex::control_data().Line(1).Col(5)).Inject());
    REQUIRE( stc->GetCurrentLine() == 0);
    REQUIRE( stc->GetCurrentPos() == 4);
    REQUIRE( wex::stc_data(stc, wex::control_data().Line(1).Col(5)).Inject());
    REQUIRE(!wex::stc_data().Control(wex::control_data().Line(1).Col(5)).Inject());
  }
}
