////////////////////////////////////////////////////////////////////////////////
// Name:      test-temp-filename.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/temp-filename.h>

TEST_CASE("wex::temp_filename")
{
  wex::temp_filename tmpx, tmpy;

  REQUIRE(!tmpx.name().empty());
  REQUIRE(!tmpy.name().empty());
  REQUIRE(tmpx.name() != tmpy.name());
}
