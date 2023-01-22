////////////////////////////////////////////////////////////////////////////////
// Name:      test-interruptible.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/interruptible.h>

TEST_CASE("wex::interruptible")
{
  wex::interruptible interruptible;

  REQUIRE(!interruptible.is_running());

  REQUIRE(interruptible.start());
  REQUIRE(interruptible.is_running());

  REQUIRE(!interruptible.start());
  REQUIRE(interruptible.is_running());

  interruptible.end();
  REQUIRE(!interruptible.is_running());

  interruptible.end();
  REQUIRE(!interruptible.is_running());
}
