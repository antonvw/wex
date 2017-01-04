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
#include <wx/extension/menucommand.h>
#include "test.h"

TEST_CASE("wxExMenuCommands")
{
  wxExMenuCommands<wxExMenuCommand> cmnds("test", {{"x"},{"y"},{"z"}});
  
  REQUIRE( cmnds.GetCommand().GetCommand()  == "x");
  REQUIRE( cmnds.GetCommands().size() == 3);
  REQUIRE( cmnds.GetFlagsKey().empty());
  REQUIRE( cmnds.GetName() == "test");
  REQUIRE(!cmnds.SetCommand(4));
  REQUIRE( cmnds.GetCommand().GetCommand()  == "x");
  REQUIRE( cmnds.SetCommand(2));
  REQUIRE( cmnds.GetCommand().GetCommand()  == "z");
}
