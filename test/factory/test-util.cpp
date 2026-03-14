////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wex/factory/frd.h>
#include <wex/factory/util.h>

#include "test.h"

TEST_CASE("wex::factory::utils")
{
  auto* stc = new wex::test::stc();

  REQUIRE(!stc->HasFocus());
  REQUIRE(frame()->get_find_focus() != stc);

  wex::bind_set_focus(stc);
  REQUIRE(!stc->HasFocus());

  stc->SetFocus();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  REQUIRE(frame()->get_find_focus() == stc);

  wex::factory::find_replace_data data;
  REQUIRE(wex::get_regex_flags(data) == 0);
}
