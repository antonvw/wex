////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-find.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/find.h>

#include "test.h"

TEST_CASE("wex::data::find")
{
  auto* stc = get_stc();
  assert(stc != nullptr);

  SUBCASE("static")
  {
    wex::data::find::recursive(true);
    REQUIRE(wex::data::find::recursive());

    wex::data::find::recursive(false);
    REQUIRE(!wex::data::find::recursive());
  }

  SUBCASE("constructor")
  {
    SUBCASE("default")
    {
      wex::data::find f;

      REQUIRE(f.is_forward());
      REQUIRE(f.stc() == nullptr);
      REQUIRE(f.text().empty());
    }

    SUBCASE("stc")
    {
      stc->DocumentStart();
      wex::data::find f(stc, std::string());

      REQUIRE(f.end_pos() == stc->GetTextLength());
      REQUIRE(f.flags() == -1);
      REQUIRE(f.is_forward());
      REQUIRE(f.line_no() == -1);
      REQUIRE(f.pos() == -1);
      REQUIRE(f.stc() == stc);
      REQUIRE(f.start_pos() == 0); // no target
      REQUIRE(f.text().empty());
    }

    SUBCASE("stream")
    {
      wex::data::find f(std::string(), 5, 6);

      REQUIRE(f.end_pos() == wxSTC_INVALID_POSITION);
      REQUIRE(f.flags() == -1);
      REQUIRE(f.is_forward());
      REQUIRE(f.line_no() == 5);
      REQUIRE(f.pos() == 6);
      REQUIRE(f.stc() == nullptr);
      REQUIRE(f.start_pos() == wxSTC_INVALID_POSITION);
      REQUIRE(f.text().empty());
    }
  }

  SUBCASE("find_margin")
  {
    stc->set_text("line 1\nline 2\nline 3\n");
    stc->DocumentStart();
    stc->SetMarginWidth(3, 100);
    stc->MarginSetText(1, "hello world");
    stc->MarginSetText(2, "hello world");

    int line = 10;

    REQUIRE(!wex::data::find(stc, "xxx").find_margin(line));
    REQUIRE(line == 10);

    REQUIRE(wex::data::find(stc, "hello").find_margin(line));
    REQUIRE(line == 1);
  }

  SUBCASE("flags")
  {
    wex::data::find f(stc, std::string());
    f.flags(100);

    REQUIRE(f.flags() == 100);
  }

  SUBCASE("statustext")
  {
    wex::data::find(stc, "xxx").statustext();
  }
}
