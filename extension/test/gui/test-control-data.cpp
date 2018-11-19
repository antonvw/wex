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
    REQUIRE( wex::control_data().col() == wex::DATA_NUMBER_NOT_SET);
    REQUIRE( wex::control_data().col(3).col() == 3);
    REQUIRE( wex::control_data().command("xx").command().command() == "xx");
    REQUIRE( wex::control_data().find("xx").find() == "xx");
    REQUIRE( wex::control_data().find("xx").find_flags() == 0);
    REQUIRE( wex::control_data().find("xx", 1).find_flags() == 1);
    REQUIRE( wex::control_data().line() == wex::DATA_NUMBER_NOT_SET);
    REQUIRE( wex::control_data().line(-1).line() == -1);
    REQUIRE( wex::control_data().line(3).line() == 3);
    REQUIRE(!wex::control_data().is_required());
    REQUIRE( wex::control_data().is_required(true).is_required());
    wex::control_data data(wex::control_data().line(3));
    data.reset();
    REQUIRE( data.line() == wex::DATA_NUMBER_NOT_SET);
    REQUIRE( wex::control_data().validator() == nullptr);
  }
  
  SUBCASE("inject")
  {
    REQUIRE(!wex::control_data().inject());
    REQUIRE(!wex::control_data().line(1).col(5).inject());
  }
}
