////////////////////////////////////////////////////////////////////////////////
// Name:      test-findbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../src/ui/findbar.h"
#include "test.h"

TEST_CASE("wex::find_bar")
{
  SUBCASE("constructor")
  {
    wex::find_bar fb(frame(), wex::data::window());

    REQUIRE(!fb.find());
    REQUIRE(fb.get_text().empty());
  }

  SUBCASE("find")
  {
    wex::find_bar fb(frame(), wex::data::window());
    fb.set_text("text");
    get_stc()->set_text("text");

    REQUIRE(!fb.find());
  }
}
