////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-vector.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/item-vector.h>
#include <wex/ui/item.h>

TEST_SUITE_BEGIN("wex::item");

TEST_CASE("wex::item_vector")
{
  const std::vector<wex::item> v({{"spin1", 0, 10, 0}});

  wex::item_vector iv(&v);

  REQUIRE(iv.find<int>("spin1") == 0);
}

TEST_SUITE_END();
