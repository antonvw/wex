////////////////////////////////////////////////////////////////////////////////
// Name:      test-auto_complete.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/auto-complete.h>

#include "test.h"

TEST_CASE("wex::auto_complete" * doctest::may_fail())
{
  auto* stc = new wex::stc(wex::test::get_path("test.h"));
  stc->SetFocus();
  frame()->pane_add(stc);
  wex::auto_complete ac(stc);

  ac.use(true);

  REQUIRE(ac.use());
  REQUIRE(!ac.activate(std::string()));
  REQUIRE(ac.activate("test_app"));
  REQUIRE(!ac.on_char(WXK_BACK));
  REQUIRE(ac.on_char('x'));
  REQUIRE(ac.on_char(WXK_BACK));
  REQUIRE(!ac.on_char(WXK_BACK));

  ac.use(false);

  REQUIRE(!ac.use());
  REQUIRE(!ac.on_char('x'));

  ac.clear();
}
