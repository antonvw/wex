////////////////////////////////////////////////////////////////////////////////
// Name:      test-control-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/control-data.h>
#include "test.h"

TEST_CASE("wex::control_data")
{
  SUBCASE("Constructor")
  {
    REQUIRE( wex::control_data().Col() == DATA_NUMBER_NOT_SET);
    REQUIRE( wex::control_data().Col(3).Col() == 3);
    REQUIRE( wex::control_data().Command("xx").Command().Command() == "xx");
    REQUIRE( wex::control_data().Find("xx").Find() == "xx");
    REQUIRE( wex::control_data().Find("xx").FindFlags() == 0);
    REQUIRE( wex::control_data().Find("xx", 1).FindFlags() == 1);
    REQUIRE( wex::control_data().Line() == DATA_NUMBER_NOT_SET);
    REQUIRE( wex::control_data().Line(-1).Line() == -1);
    REQUIRE( wex::control_data().Line(3).Line() == 3);
    REQUIRE(!wex::control_data().Required());
    REQUIRE( wex::control_data().Required(true).Required());
    wex::control_data data(wex::control_data().Line(3));
    data.Reset();
    REQUIRE( data.Line() == DATA_NUMBER_NOT_SET);
    REQUIRE( wex::control_data().Validator() == nullptr);
  }
  
  SUBCASE("Inject")
  {
    REQUIRE(!wex::control_data().Inject());
    REQUIRE(!wex::control_data().Line(1).Col(5).Inject());
  }
}
