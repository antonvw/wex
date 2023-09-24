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
  SUBCASE("default-constructor")
  {
    wex::data::stc data;

    REQUIRE(data.control().line() == 0);
    REQUIRE(data.head_path().empty());
    REQUIRE(data.recent());
    REQUIRE(!data.event(true).event().is_pos_at_end());
    REQUIRE(!data.event(true).event().is_synced());
    REQUIRE(!data.event(true).event().is_synced_log());
    REQUIRE(data.control(wex::data::control().col(3)).control().col() == 3);
    REQUIRE(data.indicator_no() == wex::data::stc::IND_LINE);
    REQUIRE(
      data.indicator_no(wex::data::stc::IND_DEBUG).indicator_no() ==
      wex::data::stc::IND_DEBUG);
    REQUIRE(
      data.flags(wex::data::stc::WIN_READ_ONLY).flags() ==
      wex::data::stc::WIN_READ_ONLY);
    REQUIRE(
      data.flags(wex::data::stc::WIN_READ_ONLY)
        .flags(wex::data::stc::WIN_HEX, wex::data::control::OR)
        .flags() != wex::data::stc::WIN_READ_ONLY);
    REQUIRE(data.menu().test(wex::data::stc::MENU_VCS));
  }

  SUBCASE("control-constructor")
  {
    REQUIRE(wex::data::stc(wex::data::control().col(3)).control().col() == 3);
    REQUIRE(
      wex::data::stc(wex::data::window().name("XX")).window().name() == "XX");
  }

  SUBCASE("window-constructor")
  {
    // TODO
  }

  SUBCASE("inject")
  {
    wex::data::stc data;
    auto*          stc = get_stc();
    data.set_stc(stc);
    stc->set_text("line 1\nline 2\nline 3\n");
    REQUIRE(data.control(wex::data::control().line(1).col(5)).inject());
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(stc->GetCurrentPos() == 4);
    REQUIRE(data.control(wex::data::control().line(1).col(5)).inject());
    REQUIRE(
      !wex::data::stc().control(wex::data::control().line(1).col(5)).inject());
  }

  SUBCASE("set")
  {
    wex::data::stc data;
    data.set_stc(get_stc());
    get_stc()->DocumentEnd();

    data.head_path(wex::path("head"));
    REQUIRE(data.get_stc() == get_stc());
    REQUIRE(data.head_path().string() == "head");
    REQUIRE(data.event(true).event().is_pos_at_end());

    data.recent(false);
    REQUIRE(!data.recent());
  }
}
