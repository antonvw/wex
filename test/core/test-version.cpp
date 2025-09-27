////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/version.h>
#include <wex/test/test.h>

TEST_CASE("wex::version_info")
{

  SECTION("default-constructor")
  {
    REQUIRE(!wex::version_info().get().empty());
  }

  SECTION("constructor")
  {
    const wex::version_info info(
      {"hello", 5, 6, 0, 0, "test-version", "wex-copyright"});

    REQUIRE(info.copyright() == "wex-copyright");
    REQUIRE(info.description() == "test-version");
    REQUIRE(info.get() == "hello 5.6");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_NAME)) == "5.6");
  }

  SECTION("constructor-micro")
  {
    const wex::version_info info({"hello", 5, 6, 7, 0});

    REQUIRE(info.get() == "hello 5.6.7");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_NAME)) == "5.6.7");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_MICRO)) == "hello 5.6");
  }

  SECTION("constructor-revision")
  {
    const wex::version_info info({"hello", 5, 6, 7, 1});

    REQUIRE(info.get() == "hello 5.6.7.1");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_NAME)) == "5.6.7.1");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_MICRO)) == "hello 5.6");
  }

  SECTION("constructor-no-micro-revision")
  {
    const wex::version_info info({"hello", 5, 6, 0, 15});

    REQUIRE(info.get() == "hello 5.6.0.15");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_NAME)) == "5.6.0.15");
    REQUIRE(
      info.get(wex::version_info::exclude_t().set(
        wex::version_info::EXCLUDE_MICRO)) == "hello 5.6");
  }

  SECTION("get_version_info")
  {
    REQUIRE(!wex::get_version_info().get().empty());
  }
}
