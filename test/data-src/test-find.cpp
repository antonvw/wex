////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-find.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/find.h>

#include "test.h"

TEST_CASE("wex::data::find")
{
  auto* stc = get_stc();
  assert(stc != nullptr);

  SUBCASE("constructor")
  {
    stc->DocumentStart();

    wex::data::find::recursive(true);
    REQUIRE(wex::data::find::recursive());

    wex::data::find::recursive(false);
    REQUIRE(!wex::data::find::recursive());

    wex::data::find f(stc, std::string());

    REQUIRE(f.start_pos() == 0);
    REQUIRE(f.end_pos() == stc->GetTextLength());
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
}
