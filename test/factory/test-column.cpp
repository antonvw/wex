////////////////////////////////////////////////////////////////////////////////
// Name:      test-column.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/listview.h>
#include <wex/test/test.h>

TEST_CASE("wex::column")
{
  SUBCASE("constructor")
  {
    wex::column c;
    REQUIRE(c.GetText().empty());
    REQUIRE(c.type() == wex::column::INVALID);
    REQUIRE(!c.is_sorted_ascending());

    c.set_is_sorted_ascending(wex::SORT_ASCENDING);
    REQUIRE(c.is_sorted_ascending());
  }

  SUBCASE("constructor-2")
  {
    wex::column c("hello");
    REQUIRE(c.GetText() == "hello");
    REQUIRE(c.type() == wex::column::INT);
  }
}
