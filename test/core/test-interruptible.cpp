////////////////////////////////////////////////////////////////////////////////
// Name:      test-interruptible.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/interruptible.h>
#include <wex/test/test.h>

TEST_CASE("wex::interruptible")
{
  wex::interruptible interruptible;

  REQUIRE(!interruptible.is_running());

  REQUIRE(interruptible.start());
  REQUIRE(interruptible.is_running());

  REQUIRE(!interruptible.start());
  REQUIRE(interruptible.is_running());

  REQUIRE(interruptible.end());
  REQUIRE(!interruptible.is_running());

  REQUIRE(!interruptible.end());
  REQUIRE(!interruptible.is_running());
}
