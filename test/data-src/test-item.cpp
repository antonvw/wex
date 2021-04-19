////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/item.h>

#include "../test.h"

TEST_CASE("wex::data::item")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::data::item().columns() == 1);
    REQUIRE(wex::data::item(wex::data::control().is_required(true))
              .control()
              .is_required());
    REQUIRE(wex::data::item().label_type() == wex::data::item::LABEL_LEFT);
    REQUIRE(wex::data::item().image_list() == nullptr);
  }
}
