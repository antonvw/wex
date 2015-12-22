////////////////////////////////////////////////////////////////////////////////
// Name:      test-interruptable.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/interruptable.h>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExInterruptable" ) 
{
  wxExInterruptable interruptable;
  
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
