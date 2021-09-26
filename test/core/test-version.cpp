////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/version.h>

TEST_CASE("wex::version")
{

  SUBCASE("default-constructor")
  {
    REQUIRE(!wex::version_info().get().empty());
  }

  SUBCASE("constructor")
  {
    const wex::version_info info(
      {"hello", 5, 6, 7, "test-version", "wex-copyright"});

    REQUIRE(info.copyright() == "wex-copyright");
    REQUIRE(info.description() == "test-version");
    REQUIRE(info.get() == "5.06.7");
  }

  SUBCASE("get_version_info")
  {
    REQUIRE(!wex::get_version_info().get().empty());
  }
}
