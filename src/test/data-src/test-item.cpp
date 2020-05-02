////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/item-data.h>

TEST_CASE("wex::item_data")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::item_data().columns() == 1);
    REQUIRE(wex::item_data(wex::control_data().is_required(true))
              .control()
              .is_required());
    REQUIRE(wex::item_data().label_type() == wex::item_data::LABEL_LEFT);
    REQUIRE(wex::item_data().image_list() == nullptr);
  }
}
