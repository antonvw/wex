////////////////////////////////////////////////////////////////////////////////
// Name:      test-log.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/log.h>

TEST_CASE("wex::log")
{
  SUBCASE("default constructor")
  {
    REQUIRE(wex::log().get().empty());
    wex::log() << "default constructor";
  }

  SUBCASE("debug")
  {
    std::stringstream ss;
    ss << "the great white";

    wex::log log(wex::log::debug("debug"));
    log << ss << "is white";
    log << std::stringstream("is hungry") << "eats" << 25 << "fish";

    REQUIRE(log.get().find("debug") == 0);
    REQUIRE(log.get().find("is hungry") != std::string::npos);
    REQUIRE(log.get().find(" eats ") != std::string::npos);
    REQUIRE(log.get().find("25") != std::string::npos);
  }

  SUBCASE("info") { wex::log::info() << "info"; }

  SUBCASE("status")
  {
    wex::log::status() << wex::test::get_path("test.h");
    wex::log::status() << "hello world";
    wex::log::status("hello") << "hello world";
  }

  SUBCASE("trace")
  {
    wex::log::trace("trace") << wex::test::get_path("test.h");
    wex::log::trace("trace") << "hello world";
  }

  SUBCASE("warning") { wex::log::warning("warning") << "hello"; }
}
