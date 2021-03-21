////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/accelerators.h>

TEST_CASE("wex::accelerators")
{
  wex::accelerators accel({{wxACCEL_CTRL, '=', 100}, {wxACCEL_CTRL, '-', 101}});

  REQUIRE(accel.size() == 2);

  accel.set(del_frame());

  wex::accelerators debug(
    {{wxACCEL_CTRL, '=', 100}, {wxACCEL_CTRL, '-', 101}},
    true);

  // debug menu is not loaded
  REQUIRE(accel.size() == 2);
}
