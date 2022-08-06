////////////////////////////////////////////////////////////////////////////////
// Name:      test-log.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>

#include "../test.h"

TEST_CASE("wex::log")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::log().get().empty());
    wex::log() << "default constructor";

    REQUIRE(wex::log("hello world").get() == "hello world");
  }

  SUBCASE("level")
  {
    REQUIRE(wex::log::get_level() == wex::log::LEVEL_ERROR);

    wex::log::set_level(wex::log::LEVEL_TRACE);
    REQUIRE(wex::log::get_level() == wex::log::LEVEL_TRACE);

    wex::log::set_level(wex::log::level_t_def());
    REQUIRE(wex::log::get_level() == wex::log::LEVEL_ERROR);
  }

  SUBCASE("debug")
  {
    std::stringstream ss;
    ss << "the great white";

    wex::log log(wex::log::debug("debug"));
    log << ss << "is white";
    log << std::stringstream("is hungry") << "eats" << 25 << "fish";

    REQUIRE(log.get().starts_with("debug"));
    REQUIRE(log.get().find("is hungry") != std::string::npos);
    REQUIRE(log.get().find(" eats ") != std::string::npos);
    REQUIRE(log.get().find("25") != std::string::npos);
  }

  SUBCASE("info")
  {
    wex::log::info() << "info";
  }

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

  SUBCASE("warning")
  {
    wex::log::warning("warning") << "hello";
  }
}

TEST_CASE("wex::log_none")
{
  const auto level(wex::log::get_level());

  {
    wex::log_none off;
    REQUIRE(wex::log::get_level() == wex::log::LEVEL_OFF);
  }

  REQUIRE(wex::log::get_level() == level);
}
