////////////////////////////////////////////////////////////////////////////////
// Name:      test-addressrange-mark.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>

#include "../src/ex/addressrange-mark.h"
#include "test.h"

TEST_CASE("wex::addressrange_mark", "[!mayfail]")
{
  auto* stc = get_stc();

  stc->set_text("hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy");

  auto* ex = new wex::ex(stc);

  SECTION("constructor")
  {
    const wex::addressrange ar(ex, "1,2");
    auto* arm = new wex::addressrange_mark(ar, wex::data::substitute());

    REQUIRE(arm->set());
    REQUIRE(arm->marker_begin() == 0);
    REQUIRE(arm->marker_target() == 0);
    REQUIRE(arm->marker_end() == 1);

#ifndef __WXGTK__
    REQUIRE(!arm->search());
#endif

    REQUIRE(arm->update());

    arm->end();
    arm->end(false);

    delete arm;

    REQUIRE(ex->marker_line('x') == wex::LINE_NUMBER_UNKNOWN);
    REQUIRE(ex->marker_line('y') == wex::LINE_NUMBER_UNKNOWN);
    REQUIRE(ex->marker_line('z') == wex::LINE_NUMBER_UNKNOWN);
    REQUIRE(ex->marker_line('T') == wex::LINE_NUMBER_UNKNOWN);
    REQUIRE(ex->marker_line('#') == wex::LINE_NUMBER_UNKNOWN);
    REQUIRE(ex->marker_line('$') == wex::LINE_NUMBER_UNKNOWN);
  }

  delete ex;
}
