////////////////////////////////////////////////////////////////////////////////
// Name:      test-auto_complete.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/auto-complete.h>
#include <wex/stc.h>

TEST_CASE("wex::auto_complete" * doctest::may_fail())
{
  auto* stc = new wex::stc(wex::test::get_path("test.h"));
  stc->SetFocus();
  wex::test::add_pane(frame(), stc);
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
