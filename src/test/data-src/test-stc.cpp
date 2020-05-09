////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/stc-data.h>
#include <wex/stc.h>

TEST_CASE("wex::stc_data")
{
  SUBCASE("Default constructor")
  {
    REQUIRE(wex::stc_data().control().line() == 0);
    REQUIRE(!wex::stc_data().event(true).event().pos_at_end());
    REQUIRE(!wex::stc_data().event(true).event().synced());
    REQUIRE(!wex::stc_data().event(true).event().synced_log());
    REQUIRE(
      wex::stc_data().control(wex::control_data().col(3)).control().col() == 3);
    REQUIRE(wex::stc_data().indicator_no() == wex::stc_data::IND_LINE);
    REQUIRE(
      wex::stc_data().indicator_no(wex::stc_data::IND_DEBUG).indicator_no() ==
      wex::stc_data::IND_DEBUG);
    REQUIRE(
      wex::stc_data().flags(wex::stc_data::WIN_READ_ONLY).flags() ==
      wex::stc_data::WIN_READ_ONLY);
    REQUIRE(
      wex::stc_data()
        .flags(wex::stc_data::WIN_READ_ONLY)
        .flags(wex::stc_data::WIN_HEX, wex::control_data::OR)
        .flags() != wex::stc_data::WIN_READ_ONLY);
    REQUIRE(wex::stc_data().menu().test(wex::stc_data::MENU_VCS));
  }

  SUBCASE("Constructor from other data")
  {
    REQUIRE(wex::stc_data(wex::control_data().col(3)).control().col() == 3);
    REQUIRE(
      wex::stc_data(wex::window_data().name("XX")).window().name() == "XX");
  }

  SUBCASE("Constructor from stc")
  {
    auto* stc = get_stc();
    assert(stc != nullptr);
    stc->DocumentEnd();
    REQUIRE(wex::stc_data(stc).event(true).event().pos_at_end());
  }

  SUBCASE("inject")
  {
    auto* stc = get_stc();
    assert(stc != nullptr);
    stc->set_text("line 1\nline 2\nline 3\n");
    REQUIRE(
      wex::stc_data(stc).control(wex::control_data().line(1).col(5)).inject());
    REQUIRE(stc->GetCurrentLine() == 0);
    REQUIRE(stc->GetCurrentPos() == 4);
    REQUIRE(wex::stc_data(stc, wex::control_data().line(1).col(5)).inject());
    REQUIRE(
      !wex::stc_data().control(wex::control_data().line(1).col(5)).inject());
  }
}
