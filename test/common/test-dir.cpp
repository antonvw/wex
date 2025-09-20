////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wex/common/dir.h>
#include <wex/common/tool.h>
#include <wex/factory/defs.h>

#include "test.h"

void test_files(const std::string& spec, size_t count, bool hidden = false)
{
  auto type = wex::data::dir::type_t().set(wex::data::dir::FILES);

  if (hidden)
  {
    type.set(wex::data::dir::HIDDEN);
  }

  wex::dir dir(
    wex::test::get_path(),
    wex::data::dir().file_spec(spec).type(type));

  REQUIRE(dir.find_files() == count);
  REQUIRE(!dir.get_statistics().empty());
}

TEST_CASE("wex::dir")
{
  SECTION("constructor")
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

  SECTION("find_files")
  {
    SECTION("dirs")
    {
      wex::dir dir(
        wex::path("../../"),
        wex::data::dir().dir_spec("data").type(
          wex::data::dir::type_t().set(wex::data::dir::DIRS)));

      REQUIRE(dir.data().file_spec().empty());
      REQUIRE(dir.data().dir_spec() == "data");
      REQUIRE(dir.find_files() == 1);
    }

    SECTION("flat")
    {
      test_files("*.h", 2, false);
    }

    SECTION("hidden")
    {
      test_files("*.h", 3, true);
    }

    SECTION("recursive")
    {
      wex::dir dir(wex::path("../../"), wex::data::dir().file_spec("*.h"));

      REQUIRE(dir.find_files() > 50);

      wex::dir limit(
        wex::path("../../"),
        wex::data::dir().file_spec("*.h").max_matches(25));

      REQUIRE(limit.find_files() == 25);
    }

    SECTION("thread")
    {
      wex::dir dir(
        wex::path("./"),
        wex::data::dir().file_spec("*.h"),
        get_listview());

      REQUIRE(dir.find_files(wex::tool(wex::ID_TOOL_REPORT_FIND)));

      wex::interruptible::end();

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  SECTION("get_all_files")
  {
    const auto& v(wex::get_all_files(
      std::string("./"),
      wex::data::dir().file_spec("*.txt").type(
        wex::data::dir::type_t().set(wex::data::dir::FILES))));

    REQUIRE(v.size() == 6);

    const auto& v2(wex::get_all_files(
      std::string("./"),
      wex::data::dir()
        .file_spec("*containing*")
        .type(wex::data::dir::type_t().set(wex::data::dir::FILES))));

    REQUIRE(v2.size() == 1);
    CAPTURE(v2.front());
    REQUIRE(v2.front().contains("\\ "));
  }

  SECTION("invalid")
  {
    wex::dir dir(
      wex::path("xxxx"),
      wex::data::dir().file_spec("*.h").type(wex::data::dir::FILES));

    REQUIRE(!dir.get_path().dir_exists());
  }

  wex::interruptible::end();
}
