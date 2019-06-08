////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
  wex::stc* stc = get_stc();

  SUBCASE("Constructor")
  {
    REQUIRE( wex::stc_data().control().line() == 0);
    REQUIRE( wex::stc_data().control(
      wex::control_data().col(3)).control().col() == 3);
    REQUIRE( wex::stc_data().indicator_no() == wex::stc_data::IND_LINE);
    REQUIRE( wex::stc_data().indicator_no(
      wex::stc_data::IND_DEBUG).indicator_no() == wex::stc_data::IND_DEBUG);
    REQUIRE( wex::stc_data(
      wex::control_data().col(3)).control().col() == 3);
    REQUIRE( wex::stc_data(
      wex::window_data().name("XX")).window().name() == "XX");
    REQUIRE( wex::stc_data().flags(
      wex::stc_data::WIN_READ_ONLY).flags() == wex::stc_data::WIN_READ_ONLY);
    REQUIRE( wex::stc_data().flags(
      wex::stc_data::WIN_READ_ONLY).flags(wex::stc_data::WIN_HEX, wex::control_data::OR).
      flags() != wex::stc_data::WIN_READ_ONLY);
    REQUIRE( wex::stc_data().menu().test(wex::stc_data::MENU_VCS));
  }
  
  SUBCASE("inject")
  {
    stc->set_text("line 1\nline 2\nline 3\n");
    REQUIRE( wex::stc_data(stc).control(
      wex::control_data().line(1).col(5)).inject());
    REQUIRE( stc->GetCurrentLine() == 0);
    REQUIRE( stc->GetCurrentPos() == 4);
    REQUIRE( wex::stc_data(stc, wex::control_data().line(1).col(5)).inject());
    REQUIRE(!wex::stc_data().control(
      wex::control_data().line(1).col(5)).inject());
  }
}
