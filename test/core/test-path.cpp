////////////////////////////////////////////////////////////////////////////////
// Name:      test-path.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/path.h>

#include "../test.h"

TEST_CASE("wex::path")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::path().empty());
    REQUIRE(wex::path("xxx").string() == "xxx");
    REQUIRE(wex::path(wex::path("yyy")).string() == "yyy");
    wex::path fn = wex::test::get_path("test.h");
    REQUIRE(wex::path(fn).fullname() == "test.h");
    REQUIRE(wex::path("..").is_relative());
    REQUIRE(!wex::path("..").is_absolute());
    REQUIRE(wex::path("xx") == wex::path("xx"));
    REQUIRE(wex::path("xx") != wex::path("xy"));
    REQUIRE(!wex::path().original().empty());
    REQUIRE(!wex::path().current().empty());
  }

  SUBCASE("basic")
  {
    wex::path path(wex::test::get_path("test.h"));

    REQUIRE(!path.dir_exists());
    REQUIRE(path.file_exists());
    REQUIRE(path.extension() == ".h");
    REQUIRE(path.fullname() == "test.h");
    REQUIRE(!path.empty());
    REQUIRE(path.log().str().find("test.h") != std::string::npos);
    REQUIRE(path.name() == "test");
    REQUIRE(!path.get_path().empty());
    REQUIRE(!path.paths().empty());
    REQUIRE(path.stat().is_ok());
    REQUIRE(!path.is_readonly());

    REQUIRE(
      path.append(wex::path("error")).string().find("error") !=
      std::string::npos);

    path.replace_filename("xxx");

    REQUIRE(!wex::path("XXXXX").stat().is_ok());

    REQUIRE(wex::path("XXXXX").make_absolute().fullname() == "XXXXX");
    REQUIRE(wex::path("XXXXX").make_absolute().string() != "XXXXX");
  }

  SUBCASE("mime")
  {
    REQUIRE(!wex::path("XXXXX").open_mime());

#ifdef __WXOSX__
    REQUIRE(wex::path("test.md").open_mime());
#endif
  }

  SUBCASE("timing")
  {
    const int       max = 1000;
    const wex::path exfile(wex::test::get_path("test.h"));
    const auto      ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.stat().is_readonly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - ex_start);
    const wex::path file(wex::test::get_path("test.h"));
    const auto      wx_start = std::chrono::system_clock::now();

    for (int j = 0; j < max; j++)
    {
      REQUIRE(!file.is_readonly());
    }

    const auto wx_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - wx_start);

    CHECK_LT(ex_milli.count(), 1000);
    CHECK_LT(wx_milli.count(), 1000);
  }
}
