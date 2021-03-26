////////////////////////////////////////////////////////////////////////////////
// Name:      test-textctrl.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/textctrl.h>

#include "test.h"

TEST_CASE("wex::textctrl")
{
  auto* tc = new wex::textctrl(frame());
  frame()->pane_add(tc->control());

  REQUIRE(tc->stc() == nullptr);
  REQUIRE(tc->get_frame() == frame());
  REQUIRE(tc->get_text().empty());

  tc->set_text("xyz");
  REQUIRE(tc->get_text() == "xyz");

  tc->set_text("abc");
  REQUIRE(tc->get_text() == "abc");

  auto* stc = get_stc();

  REQUIRE(!tc->set_stc(stc, "xxx"));
  REQUIRE(tc->set_stc(stc, "/abc"));
}
