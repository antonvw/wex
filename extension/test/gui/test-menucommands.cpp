////////////////////////////////////////////////////////////////////////////////
// Name:      test-menucommands.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/menucommands.h>
#include "test.h"

TEST_CASE("wxExMenuCommands", "[menu]")
{
  wxExMenuCommands(
  wxExMenuCommand add("a&dd");
  
  REQUIRE( cmnds.GetCommand() > 0));
  REQUIRE(!cmnds.GetCommands().empty()));
  REQUIRE(!cmnds.GetFlagsKey().empty()));
  REQUIRE(!cmnds.GetName().empty()));
  REQUIRE( cmnds.SetCommand(4));
}
