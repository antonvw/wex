////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/defs.h>
#include <wex/dir.h>
#include <wex/tool.h>

#include "test.h"

TEST_CASE("wex::dir")
{
  SUBCASE("constructor")
  {
    wex::dir dir(wex::test::get_path());

    REQUIRE(dir.data().dir_spec().empty());
    REQUIRE(dir.data().file_spec().empty());
    REQUIRE(dir.get_statistics().empty());
    REQUIRE(dir.data().type().test(wex::data::dir::FILES));
    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.handler() == nullptr);

    wex::dir dir2(wex::test::get_path(), wex::data::dir().file_spec("*.h"));
    REQUIRE(dir2.data().file_spec() == "*.h");
  }

  SUBCASE("find_files")
  {
    SUBCASE("dirs")
    {
      wex::dir dir(
        wex::path("../../"),
        wex::data::dir().dir_spec("data").type(
          wex::data::dir::type_t().set(wex::data::dir::DIRS)));

      REQUIRE(dir.data().file_spec().empty());
      REQUIRE(dir.data().dir_spec() == "data");
      REQUIRE(dir.find_files() == 1);
    }

    SUBCASE("flat")
    {
      wex::dir dir(
        wex::test::get_path(),
        wex::data::dir().file_spec("*.h").type(
          wex::data::dir::type_t().set(wex::data::dir::FILES)));

      REQUIRE(dir.find_files() == 2);
      REQUIRE(!dir.get_statistics().empty());
    }

    SUBCASE("hidden")
    {
      wex::dir dir(
        wex::test::get_path(),
        wex::data::dir().file_spec("*.h").type(wex::data::dir::type_t()
                                                 .set(wex::data::dir::FILES)
                                                 .set(wex::data::dir::HIDDEN)));

      REQUIRE(dir.find_files() == 3);
      REQUIRE(!dir.get_statistics().empty());
    }

    SUBCASE("recursive")
    {
      wex::dir dir(wex::path("../../"), wex::data::dir().file_spec("*.h"));

      REQUIRE(dir.find_files() > 50);

      wex::dir limit(
        wex::path("../../"),
        wex::data::dir().file_spec("*.h").max_matches(25));

      REQUIRE(limit.find_files() == 25);
    }

    SUBCASE("thread")
    {
      wex::dir dir(
        wex::path("./"),
        wex::data::dir().file_spec("*.h"),
        get_listview());

      REQUIRE(dir.find_files(wex::tool(wex::ID_TOOL_REPORT_FIND)));

      wex::interruptible::stop();
      wxYield();
    }
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

  SUBCASE("invalid")
  {
    wex::dir dir(
      wex::path("xxxx"),
      wex::data::dir().file_spec("*.h").type(wex::data::dir::FILES));

    REQUIRE(!dir.get_path().dir_exists());
  }

  wex::interruptible::stop();
}
