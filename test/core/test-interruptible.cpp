////////////////////////////////////////////////////////////////////////////////
// Name:      test-interruptible.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/interruptible.h>

TEST_CASE("wex::interruptible")
{
  wex::interruptible interruptible;

  REQUIRE(!interruptible.is_running());
  REQUIRE(!interruptible.is_cancelled());
  REQUIRE(!interruptible.cancel());

  interruptible.start();
  REQUIRE(interruptible.is_running());
  REQUIRE(!interruptible.is_cancelled());

  interruptible.stop();
  REQUIRE(!interruptible.is_running());
  REQUIRE(!interruptible.is_cancelled());
  REQUIRE(!interruptible.cancel());

  interruptible.start();
  REQUIRE(interruptible.cancel());
  REQUIRE(!interruptible.is_running());
  REQUIRE(interruptible.is_cancelled());
}
