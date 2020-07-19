////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-find.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/find-data.h>
#include <wex/stc.h>

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

    wex::data::find f(stc);

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

    wex::data::find f(stc);
    int             line = 10;

    REQUIRE(!f.find_margin("xxx", line));
    REQUIRE(line == 10);

    REQUIRE(f.find_margin("hello", line));
    REQUIRE(line == 1);
  }
}
