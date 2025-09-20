////////////////////////////////////////////////////////////////////////////////
// Name:      test-findbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../src/ui/findbar.h"
#include "test.h"
#include <wex/ui/frd.h>

TEST_CASE("wex::find_bar")
{
  auto* fb = new wex::find_bar(frame(), wex::data::window());

  SECTION("constructor")
  {
    REQUIRE(!fb->find());
    REQUIRE(fb->get_text().empty());
  }

  SECTION("find")
  {
    fb->set_stc(get_stc());
    get_stc()->set_text("text1\ntext2\ntext3\n");
    fb->set_text("text");
    fb->control()->SetFocus();
    REQUIRE(fb->stc() != nullptr);

    wex::find_replace_data::get()->set_match_word(true);
    REQUIRE(!fb->find());

    wex::find_replace_data::get()->set_match_word(false);
    REQUIRE(fb->find());
  }

  SECTION("find_on_enter")
  {
    fb->set_text("text");
    get_stc()->set_text("text");

    REQUIRE(fb->find_on_enter());
  }
}
