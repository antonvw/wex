////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-build.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/item-build.h>

#include "test.h"

TEST_CASE("wex::add_header")
{
  const auto& header(wex::add_header({"1", "2", "3", "4", "5"}));

  REQUIRE(header.size() == 5);
  REQUIRE(header.front().label() == "1");
  REQUIRE(header.front().type() == wex::item::STATICTEXT);
}
