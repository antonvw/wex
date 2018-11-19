////////////////////////////////////////////////////////////////////////////////
// Name:      test-interruptable.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/interruptable.h>
#include "../test.h"

TEST_CASE( "wex::interruptable" ) 
{
  wex::interruptable interruptable;
  
  REQUIRE(!interruptable.is_running());
  REQUIRE(!interruptable.is_cancelled());
  REQUIRE(!interruptable.cancel());
  
  interruptable.start();
  REQUIRE( interruptable.is_running());
  REQUIRE(!interruptable.is_cancelled());
  
  interruptable.stop();
  REQUIRE(!interruptable.is_running());
  REQUIRE(!interruptable.is_cancelled());
  REQUIRE(!interruptable.cancel());
  
  interruptable.start();
  REQUIRE( interruptable.cancel());
  REQUIRE(!interruptable.is_running());
  REQUIRE( interruptable.is_cancelled());
}
