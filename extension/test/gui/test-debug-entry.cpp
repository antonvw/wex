////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug-entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/debug-entry.h>
#include "test.h"

TEST_CASE("wex::debug_entry")
{
  SUBCASE("Default constructor")
  {
    REQUIRE( wex::debug_entry().get_commands().empty());
  }
  
  SUBCASE("Constructor using xml")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<debug name=\"gdb\" pos-begin=\"11\" pos-end=\"20\"></debug>"));

    wex::debug_entry entry(doc.document_element());
    REQUIRE( entry.name() == "gdb");
  }
}
