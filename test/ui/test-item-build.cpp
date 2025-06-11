////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-build.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/item-build.h>

#include "test.h"

TEST_CASE("wex::add_combobox_with_max")
{
  const auto& combo_max(wex::add_combobox_with_max("combox", "max"));

  REQUIRE(combo_max.label().empty());
  REQUIRE(combo_max.type() == wex::item::GROUP);
}

TEST_CASE("wex::add_find_text")
{
  const auto& find(wex::add_find_text());

  REQUIRE(find.label() == "fif.Find what");
  REQUIRE(find.type() == wex::item::COMBOBOX);
}

TEST_CASE("wex::add_header")
{
  const auto& header(wex::add_header({"1", "2", "3", "4", "5"}));

  REQUIRE(header.size() == 5);
  REQUIRE(header.front().label() == "1");
  REQUIRE(header.front().type() == wex::item::STATICTEXT);
}
