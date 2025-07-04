////////////////////////////////////////////////////////////////////////////////
// Name:      test-scope.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/stc/bind.h>

#include "../src/stc/scope.h"

#include "test.h"

TEST_CASE("wex::scope")
{
  auto* stc = new wex::stc();
  frame()->pane_add(stc);
  stc->get_vi().command("\x1b");

  SECTION("constructor")
  {
    wex::scope scope(stc);

    REQUIRE(scope.end());
    REQUIRE(!scope.find(""));
    REQUIRE(!scope.find("xyz"));
  }
}
