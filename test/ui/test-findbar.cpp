////////////////////////////////////////////////////////////////////////////////
// Name:      test-findbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../src/ui/findbar.h"
#include "test.h"

TEST_CASE("wex::find_bar")
{
  auto* fb = new wex::find_bar(frame(), wex::data::window());

  SUBCASE("constructor")
  {
    REQUIRE(!fb->find());
    REQUIRE(fb->get_text().empty());
  }

  SUBCASE("find")
  {
    get_stc()->set_text("text1\ntext2\ntext3\n");
    fb->set_text("text");
    fb->control()->SetFocus();

    REQUIRE(fb->stc() != nullptr);
    REQUIRE(fb->find());
  }

  SUBCASE("find_on_enter")
  {
    fb->set_text("text");
    get_stc()->set_text("text");

    REQUIRE(fb->find_on_enter());
  }
}
