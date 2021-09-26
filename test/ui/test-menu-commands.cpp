////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu-commands.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/menu-command.h>
#include <wex/ui/menu-commands.h>

TEST_CASE("wex::menu_commands")
{
  pugi::xml_document doc;
  doc.load_string("<command> add </command>");
  const wex::menu_command cmd(doc.document_element());

  wex::menu_commands<wex::menu_command> cmnds("test", {{}, {}, cmd});

  REQUIRE(cmnds.get_command().get_command().empty());
  REQUIRE(cmnds.get_commands().size() == 3);
  REQUIRE(cmnds.find("add").get_command() == "add");
  REQUIRE(cmnds.find("xxx").get_command().empty());
  REQUIRE(cmnds.flags_key().empty());
  REQUIRE(cmnds.name() == "test");
  REQUIRE(!cmnds.set_command(4));
  REQUIRE(cmnds.set_command(2));
}
