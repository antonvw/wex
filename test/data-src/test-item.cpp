////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/data/item.h>
#include <wex/test/test.h>

TEST_CASE("wex::data::item")
{
  SECTION("constructor")
  {
    REQUIRE(wex::data::item().apply() == nullptr);
    REQUIRE(wex::data::item().columns() == 1);
    REQUIRE(wex::data::item().image_list() == nullptr);
    REQUIRE(wex::data::item().is_persistent());
    REQUIRE(!wex::data::item().is_readonly());
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

  SECTION("operator")
  {
    wex::data::item item;
    item.is_persistent(false).is_readonly(true).is_regex(true).apply(
      [=](wxWindow* user, const std::any& value, bool save)
      {
        wex::log::status("lambda") << "this is a lambda\n";
      });

    const wex::data::item copy(item);
    REQUIRE(copy.apply() != nullptr);
    REQUIRE(!copy.is_persistent());
    REQUIRE(copy.is_readonly());
    REQUIRE(copy.is_regex());
  }

  SECTION("set")
  {
    wex::data::item item;
    item.is_regex(true);
    item.label_type(wex::data::item::LABEL_NONE);

    REQUIRE(item.is_regex());
    REQUIRE(item.label_type() == wex::data::item::LABEL_NONE);
  }
}
