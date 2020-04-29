////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu_commands.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/menucommand.h>
#include <wex/menucommands.h>
#include "test.h"

TEST_CASE("wex::menu_commands")
{
  wex::menu_commands<wex::menu_command> cmnds("test", {{},{},{}});
  
  REQUIRE( cmnds.get_command().get_command().empty());
  REQUIRE( cmnds.get_commands().size() == 3);
  REQUIRE( cmnds.flags_key().empty());
  REQUIRE( cmnds.name() == "test");
  REQUIRE(!cmnds.set_command(4));
  REQUIRE( cmnds.set_command(2));
}
