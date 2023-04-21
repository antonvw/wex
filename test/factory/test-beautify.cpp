////////////////////////////////////////////////////////////////////////////////
// Name:      factory/test-beautify.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/beautify.h>

#include "test.h"

TEST_CASE("wex::factory::beautify")
{
  SUBCASE("access")
  {
    REQUIRE(!wex::factory::beautify().is_active());
    REQUIRE(!wex::factory::beautify().is_auto());
    REQUIRE(!wex::factory::beautify().is_supported(wex::path("xxx.pas")));
    REQUIRE(wex::factory::beautify().is_supported(wex::path("xxx.cpp")));
    REQUIRE(!wex::factory::beautify().list().empty());
    REQUIRE(wex::factory::beautify().name().empty());
  }

  wex::config("stc.Beautifier").set(wex::config::strings_t{{"clang-format"}});

  SUBCASE("file")
  {
    REQUIRE(wex::factory::beautify().is_active());
    REQUIRE(wex::factory::beautify().name() == "clang-format");
    REQUIRE(!wex::factory::beautify().file(wex::path("xxxx")));
    REQUIRE(!wex::factory::beautify().file(wex::path("test.md")));
  }

  wex::config("stc.Beautifier").set(wex::config::strings_t{{""}});
}
