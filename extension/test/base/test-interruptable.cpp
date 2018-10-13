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
#include <wx/extension/interruptable.h>
#include "../test.h"

TEST_CASE( "wex::interruptable" ) 
{
  wex::interruptable interruptable;
  
  REQUIRE(!interruptable.Running());
  REQUIRE(!interruptable.Cancelled());
  REQUIRE(!interruptable.Cancel());
  
  interruptable.Start();
  REQUIRE( interruptable.Running());
  REQUIRE(!interruptable.Cancelled());
  
  interruptable.Stop();
  REQUIRE(!interruptable.Running());
  REQUIRE(!interruptable.Cancelled());
  REQUIRE(!interruptable.Cancel());
  
  interruptable.Start();
  REQUIRE( interruptable.Cancel());
  REQUIRE(!interruptable.Running());
  REQUIRE( interruptable.Cancelled());
}
