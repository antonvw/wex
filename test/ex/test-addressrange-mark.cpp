////////////////////////////////////////////////////////////////////////////////
// Name:      test-addressrange-mark.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>

#include "../src/ex/addressrange-mark.h"
#include "test.h"

TEST_CASE("wex::addressrange_mark")
{
  auto* stc = get_stc();

  stc->set_text("hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy");

  auto* ex = new wex::ex(stc);

  SUBCASE("constructor")
  {
    wex::addressrange ar(ex, "1,2");
    auto* arm = new wex::addressrange_mark(ar, wex::data::substitute());

    REQUIRE(arm->set());
    REQUIRE(ex->marker_line('#') == 0);
    REQUIRE(ex->marker_line('$') == 1);
    REQUIRE(!arm->search());

    arm->end();
    arm->end(false);

    delete arm;

    REQUIRE(ex->marker_line('#') == wex::LINE_NUMBER_UNKNOWN);
    REQUIRE(ex->marker_line('$') == wex::LINE_NUMBER_UNKNOWN);
  }

  delete ex;
}
