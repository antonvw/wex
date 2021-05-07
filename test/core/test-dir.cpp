////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/bind.h>
#include <wex/defs.h>
#include <wex/dir.h>
#include <wex/log.h>

#include "../test.h"

TEST_CASE("wex::dir")
{
  SUBCASE("not recursive")
  {
    wex::dir dir(
      wex::test::get_path(),
      wex::data::dir().file_spec("*.h").type(
        wex::data::dir::type_t().set(wex::data::dir::FILES)));

    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.data().type().test(wex::data::dir::FILES));
    REQUIRE(dir.data().file_spec() == "*.h");
    REQUIRE(dir.data().dir_spec().empty());
    REQUIRE(dir.find_files() == 2);
  }

  SUBCASE("recursive")
  {
    wex::dir dir(wex::path("../../"), wex::data::dir().file_spec("*.h"));

    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.data().file_spec() == "*.h");
    REQUIRE(dir.find_files() > 50);

    wex::dir limit(
      wex::path("../../"),
      wex::data::dir().file_spec("*.h").max_matches(25));
    REQUIRE(limit.find_files() == 25);
  }

  SUBCASE("match folders")
  {
    wex::log::trace() << wex::path::current();
    wex::dir dir(
      wex::path("../../"),
      wex::data::dir().dir_spec("data").type(
        wex::data::dir::type_t().set(wex::data::dir::DIRS)));

    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.data().file_spec().empty());
    REQUIRE(dir.data().dir_spec() == "data");
    REQUIRE(dir.find_files() == 1);
  }

  SUBCASE("invalid")
  {
    wex::dir dir(
      wex::path("xxxx"),
      wex::data::dir().file_spec("*.h").type(wex::data::dir::FILES));

    REQUIRE(!dir.get_path().dir_exists());
  }

  SUBCASE("get_all_files")
  {
    REQUIRE(
      wex::get_all_files(
        std::string("./"),
        wex::data::dir().file_spec("*.txt").type(
          wex::data::dir::type_t().set(wex::data::dir::FILES)))
        .size() == 4);
  }
}
