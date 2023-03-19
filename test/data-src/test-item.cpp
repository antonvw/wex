////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/item.h>

#include <wex/test/test.h>

TEST_CASE("wex::data::item")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::data::item().apply() == nullptr);
    REQUIRE(wex::data::item().columns() == 1);
    REQUIRE(wex::data::item().image_list() == nullptr);
    REQUIRE(!wex::data::item().is_regex());
    REQUIRE(std::any_cast<int>(wex::data::item().inc()) == 1);
    REQUIRE(wex::data::item().label_type() == wex::data::item::LABEL_LEFT);
    REQUIRE(std::any_cast<int>(wex::data::item().min()) == 0);
    REQUIRE(std::any_cast<int>(wex::data::item().max()) == 1);
    REQUIRE(wex::data::item().validate() == nullptr);

    REQUIRE(wex::data::item(wex::data::control().is_required(true))
              .control()
              .is_required());
  }

  SUBCASE("set")
  {
    wex::data::item item;
    item.is_regex(true);
    item.label_type(wex::data::item::LABEL_NONE);

    REQUIRE(item.is_regex());
    REQUIRE(item.label_type() == wex::data::item::LABEL_NONE);
  }
}
