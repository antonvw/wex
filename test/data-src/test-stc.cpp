////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/stc.h>

#include "test.h"

TEST_CASE("wex::data::stc")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::data::stc().control().line() == 0);
    REQUIRE(wex::data::stc().head_path().empty());
    REQUIRE(wex::data::stc().recent());
    REQUIRE(!wex::data::stc().event(true).event().is_pos_at_end());
    REQUIRE(!wex::data::stc().event(true).event().is_synced());
    REQUIRE(!wex::data::stc().event(true).event().is_synced_log());
    REQUIRE(
      wex::data::stc().control(wex::data::control().col(3)).control().col() ==
      3);
    REQUIRE(wex::data::stc().indicator_no() == wex::data::stc::IND_LINE);
    REQUIRE(
      wex::data::stc().indicator_no(wex::data::stc::IND_DEBUG).indicator_no() ==
      wex::data::stc::IND_DEBUG);
    REQUIRE(
      wex::data::stc().flags(wex::data::stc::WIN_READ_ONLY).flags() ==
      wex::data::stc::WIN_READ_ONLY);
    REQUIRE(
      wex::data::stc()
        .flags(wex::data::stc::WIN_READ_ONLY)
        .flags(wex::data::stc::WIN_HEX, wex::data::control::OR)
        .flags() != wex::data::stc::WIN_READ_ONLY);
    REQUIRE(wex::data::stc().menu().test(wex::data::stc::MENU_VCS));
  }

  SUBCASE("constructor-2")
  {
    REQUIRE(wex::data::stc(wex::data::control().col(3)).control().col() == 3);
    REQUIRE(
      wex::data::stc(wex::data::window().name("XX")).window().name() == "XX");
  }

  SUBCASE("constructor-3")
  {
    auto* stc = get_stc();
    stc->DocumentEnd();
    REQUIRE(wex::data::stc(stc).event(true).event().is_pos_at_end());
  }

  SUBCASE("inject")
  {
    auto* stc = get_stc();
    stc->set_text("line 1\nline 2\nline 3\n");
    REQUIRE(wex::data::stc(stc)
              .control(wex::data::control().line(1).col(5))
              .inject());
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(stc->GetCurrentPos() == 4);
    REQUIRE(wex::data::stc(stc, wex::data::control().line(1).col(5)).inject());
    REQUIRE(
      !wex::data::stc().control(wex::data::control().line(1).col(5)).inject());
  }

  SUBCASE("set")
  {
    wex::data::stc data;

    data.head_path(wex::path("head"));
    REQUIRE(data.head_path().string() == "head");

    data.recent(false);
    REQUIRE(!data.recent());
  }
}
