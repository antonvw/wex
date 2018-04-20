////////////////////////////////////////////////////////////////////////////////
// Name:      test-type-to-value.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/type-to-value.h>
#include "test.h"

TEST_CASE("wxEx")
{
  REQUIRE( wxExTypeToValue<int>("100").get() == 100);
  REQUIRE( wxExTypeToValue<int>("A").get() == 65);
  REQUIRE( wxExTypeToValue<int>(100).get() == 100);
  REQUIRE( wxExTypeToValue<int>(1).getString() == "ctrl-A");
  REQUIRE( wxExTypeToValue<int>("100").getString() == "100");
  REQUIRE( wxExTypeToValue<int>("xxx").getString() == "xxx");
  REQUIRE( wxExTypeToValue<std::string>("100").get() == "100");
  REQUIRE( wxExTypeToValue<std::string>("100").getString() == "100");
}
