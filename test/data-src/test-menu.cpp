////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-menu.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/menu.h>
#include <wex/test/test.h>

TEST_CASE("wex::data::menu")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::data::menu().art().empty());
    REQUIRE(wex::data::menu().help_text().empty());

    wex::data::menu().bind(0);
  }
}
