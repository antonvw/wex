////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-layout.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/layout.h>
#include <wx/sizer.h>

#include "test.h"

TEST_CASE("wex::data::layout")
{
  SUBCASE("constructor")
  {
    SUBCASE("cols")
    {
      wex::data::layout layout(get_listview(), 5);

      REQUIRE(!layout.is_readonly());
      REQUIRE(layout.parent() == get_listview());

      REQUIRE(!layout.sizer_grow_row());

      REQUIRE(layout.sizer_layout() == nullptr);
      REQUIRE(!layout.sizer_layout_grow_col());
      REQUIRE(!layout.sizer_layout_grow_row());
    }

    SUBCASE("cols-and-rows")
    {
      wex::data::layout layout(get_listview(), 5, 5);

      REQUIRE(!layout.is_readonly());
      REQUIRE(layout.parent() == get_listview());

      REQUIRE(layout.sizer_grow_row());
      REQUIRE(!layout.sizer_grow_row());

      REQUIRE(layout.sizer_layout() == nullptr);
      REQUIRE(!layout.sizer_layout_grow_col());
      REQUIRE(!layout.sizer_layout_grow_row());
    }
  }

  SUBCASE("set")
  {
    wex::data::layout layout(get_listview(), 4, 3);

    REQUIRE(layout.sizer_layout_create(new wxFlexGridSizer(5)));
    REQUIRE(layout.sizer_layout_create(new wxFlexGridSizer(5, 4, 0, 0)));

    REQUIRE(layout.sizer_layout() != nullptr);
    REQUIRE(layout.sizer_layout() != layout.sizer());

    REQUIRE(layout.sizer_layout_grow_row());
    REQUIRE(!layout.sizer_layout_grow_row());

    REQUIRE(layout.sizer_layout_grow_col());
    REQUIRE(!layout.sizer_layout_grow_col());
  }
}
