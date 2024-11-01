////////////////////////////////////////////////////////////////////////////////
// Name:      factory/test-beautify.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/beautify.h>
#include <wex/test/test.h>

TEST_CASE("wex::factory::beautify")
{
  SUBCASE("access")
  {
    REQUIRE(
      wex::factory::beautify(wex::path("xxx")).type() ==
      wex::factory::beautify::UNKNOWN);
    REQUIRE(
      wex::factory::beautify(wex::path("x.cmake")).type() ==
      wex::factory::beautify::CMAKE);
    REQUIRE(
      wex::factory::beautify(wex::path("xx.cpp")).type() ==
      wex::factory::beautify::SOURCE);
    REQUIRE(
      wex::factory::beautify(wex::path("xxi.robot")).type() ==
      wex::factory::beautify::ROBOTFRAMEWORK);
    REQUIRE(!wex::factory::beautify().is_active());
    REQUIRE(!wex::factory::beautify(wex::factory::beautify::CMAKE).is_active());
    REQUIRE(!wex::factory::beautify().is_auto());
    REQUIRE(!wex::factory::beautify(wex::factory::beautify::CMAKE).is_auto());
    REQUIRE(!wex::factory::beautify(wex::factory::beautify::SOURCE)
               .is_supported(wex::path("xxx.pas")));
    REQUIRE(wex::factory::beautify(wex::factory::beautify::SOURCE)
              .is_supported(wex::path("xxx.c")));
    REQUIRE(wex::factory::beautify(wex::factory::beautify::SOURCE)
              .is_supported(wex::path("xxx.cpp")));
    REQUIRE(wex::factory::beautify(wex::factory::beautify::SOURCE)
              .is_supported(wex::path("xxx.h")));
    REQUIRE(wex::factory::beautify(wex::factory::beautify::SOURCE)
              .is_supported(wex::path("xxx.hpp")));
    REQUIRE(!wex::factory::beautify().list().empty());
    REQUIRE(wex::factory::beautify().name().empty());
    REQUIRE(
      wex::factory::beautify(wex::factory::beautify::CMAKE).name().empty());
  }

  wex::config("stc.Beautifier").set(wex::config::strings_t{{"clang-format"}});
  wex::config("stc.Beautifier cmake").set(wex::config::strings_t{{"gersemi"}});
  wex::config("stc.Beautifier robotframework")
    .set(wex::config::strings_t{{"robotidy"}});

  SUBCASE("check")
  {
    wex::factory::beautify b;

    REQUIRE(!b.is_active());
    REQUIRE(!b.check(wex::path("xxx")));
    REQUIRE(b.type() == wex::factory::beautify::UNKNOWN);
    REQUIRE(b.check(wex::path("xxx.cmake")));
    REQUIRE(b.type() == wex::factory::beautify::CMAKE);
    REQUIRE(b.check(wex::path("xxx.cpp")));
    REQUIRE(b.type() == wex::factory::beautify::SOURCE);
    REQUIRE(b.check(wex::path("xxx.robot")));
    REQUIRE(b.type() == wex::factory::beautify::ROBOTFRAMEWORK);
  }

  SUBCASE("file")
  {
    REQUIRE(wex::factory::beautify(wex::factory::beautify::SOURCE).is_active());
    REQUIRE(
      wex::factory::beautify(wex::factory::beautify::SOURCE).name() ==
      "clang-format");
    REQUIRE(!wex::factory::beautify(wex::factory::beautify::SOURCE)
               .file(wex::path("xxxx")));
    REQUIRE(!wex::factory::beautify(wex::factory::beautify::SOURCE)
               .file(wex::path("test.md")));
  }

  wex::config("stc.Beautifier").set(wex::config::strings_t{{""}});
  wex::config("stc.Beautifier cmake").set(wex::config::strings_t{{""}});
  wex::config("stc.Beautifier robotframework")
    .set(wex::config::strings_t{{""}});
}
