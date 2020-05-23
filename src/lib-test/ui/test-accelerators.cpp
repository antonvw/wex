////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/accelerators.h>
#include <wex/managedframe.h>
#include "../test.h"

TEST_CASE("wex::accelerators")
{
  wex::accelerators accel({
    {wxACCEL_CTRL, '=', 100},
    {wxACCEL_CTRL, '-', 101}});
    
  REQUIRE( accel.size() == 2);

  accel.set(frame());
  
  wex::accelerators debug({
    {wxACCEL_CTRL, '=', 100},
    {wxACCEL_CTRL, '-', 101}}, true);
  
  // debug menu is not loaded
  REQUIRE( accel.size() == 2);
}
