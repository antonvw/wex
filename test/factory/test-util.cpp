////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
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
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  REQUIRE(frame()->get_find_focus() == stc);
}
