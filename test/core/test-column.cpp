////////////////////////////////////////////////////////////////////////////////
// Name:      test-column.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/listview-core.h>

TEST_CASE("wex::column")
{
  SUBCASE("default constructor")
  {
    wex::column c;
    REQUIRE(c.GetText().empty());
    REQUIRE(c.type() == wex::column::INVALID);
    REQUIRE(!c.is_sorted_ascending());

    c.set_is_sorted_ascending(wex::SORT_ASCENDING);
    REQUIRE(c.is_sorted_ascending());
  }

  SUBCASE("constructor")
  {
    wex::column c("hello");
    REQUIRE(c.GetText() == "hello");
    REQUIRE(c.type() == wex::column::INT);
  }
}
