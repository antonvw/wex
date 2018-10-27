////////////////////////////////////////////////////////////////////////////////
// Name:      test-type-to-value.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/type-to-value.h>
#include "test.h"

TEST_CASE("wex::type_to_value")
{
  REQUIRE( wex::type_to_value<int>("100").get() == 100);
  REQUIRE( wex::type_to_value<int>("A").get() == 65);
  REQUIRE( wex::type_to_value<int>(100).get() == 100);
  REQUIRE( wex::type_to_value<int>(1).getString() == "ctrl-A");
  REQUIRE( wex::type_to_value<int>("100").getString() == "100");
  REQUIRE( wex::type_to_value<int>("xxx").getString() == "xxx");
  REQUIRE( wex::type_to_value<std::string>("100").get() == "100");
  REQUIRE( wex::type_to_value<std::string>("100").getString() == "100");
}
