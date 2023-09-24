////////////////////////////////////////////////////////////////////////////////
// Name:      test-log.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/test/test.h>

#include <iostream>

TEST_CASE("wex::log")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::log().get().empty());
    wex::log() << "default constructor";

    REQUIRE(wex::log("hello world").get() == "hello world");
  }

  SUBCASE("debug")
  {
    std::stringstream ss;
    ss << "the great white";

    wex::log log(wex::log::debug("debug"));
    log << ss << "is white";
    log << std::stringstream("is hungry") << "eats" << 25 << "fish" << true
        << 'a';

    REQUIRE(log.get().starts_with("debug"));
    REQUIRE(log.get().contains("is hungry"));
    REQUIRE(log.get().contains(" eats "));
    REQUIRE(log.get().contains("25"));
    REQUIRE(!log.get().contains("\""));

    log << ss << std::string("a string");
    REQUIRE(log.get().contains("\""));
  }

  SUBCASE("info")
  {
    wex::log::info() << "info";
  }

  SUBCASE("level")
  {
    const auto current = wex::log::LEVEL_ERROR;

    wex::log::set_level(wex::log::LEVEL_TRACE);
    REQUIRE(wex::log::get_level() == wex::log::LEVEL_TRACE);

    wex::log::set_level(current);
    REQUIRE(wex::log::get_level() == current);
  }

  SUBCASE("off")
  {
    const auto current = wex::log::LEVEL_ERROR;

    {
      wex::log log("off");
      wex::log::set_level(wex::log::LEVEL_OFF);
      REQUIRE(wex::log::get_level() == wex::log::LEVEL_OFF);
      log << wex::test::get_path("test.h");

      REQUIRE(!log.get().empty());
    }

    wex::log::set_level(current);

    wex::file logfile(wex::log::path().c_str(), std::ios_base::in);
    CAPTURE(wex::log::path());
    REQUIRE(logfile.is_open());

    if (auto* text = logfile.read(); text->size() > 1)
    {
      auto pos(text->substr(0, text->size() - 2).find_last_of('\n'));
      REQUIRE(!text->substr(pos).contains("off:"));
    }
  }

  SUBCASE("status")
  {
    wex::log log(wex::log::status("status"));

    SUBCASE("path")
    {
      log << wex::test::get_path("test.h");
      log << "hello world";

      REQUIRE(log.get().starts_with("status: "));
      REQUIRE(!log.get().contains("\""));

      log << std::string("a string");
      REQUIRE(!log.get().contains("\""));
    }

    SUBCASE("string")
    {
      log << std::string("one");
      REQUIRE(log.get().starts_with("status: "));
    }
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
