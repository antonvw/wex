////////////////////////////////////////////////////////////////////////////////
// Name:      test-beautify.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/beautify.h>

TEST_CASE("wex::beautify")
{
  SUBCASE("info")
  {
    REQUIRE(!wex::beautify().is_active());
    REQUIRE(!wex::beautify().is_auto());
    REQUIRE(!wex::beautify().is_supported(wex::lexer("pascal")));
    REQUIRE(wex::beautify().is_supported(wex::lexer("cpp")));
    REQUIRE(!wex::beautify().list().empty());
  }
}
