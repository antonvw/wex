////////////////////////////////////////////////////////////////////////////////
// Name:      test-textctrl.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/managed-frame.h>
#include <wex/textctrl.h>

TEST_CASE("wex::textctrl")
{
  auto* tc = new wex::textctrl(frame());
  wex::test::add_pane(frame(), tc->control());

  REQUIRE(tc->ex() == nullptr);

  REQUIRE(tc->frame() == frame());

  REQUIRE(tc->get_text().empty());

  tc->set_text("xyz");
  REQUIRE(tc->get_text() == "xyz");

  tc->set_text("abc");
  REQUIRE(tc->get_text() == "abc");

  wex::stc* stc = get_stc();
  wex::ex*  ex  = &stc->get_vi();

  REQUIRE(!tc->set_ex(ex, "xxx"));
  REQUIRE(tc->set_ex(ex, "/abc"));
}
