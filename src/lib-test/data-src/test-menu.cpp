////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-menu.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/menu-data.h>

TEST_CASE("wex::menu_data")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::menu_data().art().empty());
    REQUIRE(wex::menu_data().help_text().empty());
    
    wex::menu_data().bind(0);
  }
}
