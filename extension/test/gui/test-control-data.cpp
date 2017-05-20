////////////////////////////////////////////////////////////////////////////////
// Name:      test-control-data.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/control-data.h>
#include "test.h"

TEST_CASE("wxExControlData")
{
  SUBCASE("Constructor")
  {
    REQUIRE( wxExControlData().Col(3).Col() == 3);
    REQUIRE( wxExControlData().Command("xx").Command() == "xx");
    REQUIRE( wxExControlData().Find("xx").Find() == "xx");
    REQUIRE( wxExControlData().Line() == 0);
    REQUIRE( wxExControlData().Line(-1).Line() == -1);
    REQUIRE( wxExControlData().Line(3).Line() == 3);
    REQUIRE( wxExControlData().Validator() == nullptr);
  }
  
  SUBCASE("Inject")
  {
    REQUIRE(!wxExControlData().Inject());
    REQUIRE(!wxExControlData().Line(1).Col(5).Inject());
  }
}
