////////////////////////////////////////////////////////////////////////////////
// Name:      test-path.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/path.h>

#include "../test.h"

#include <chrono>

bool log_contains(const wex::path::log_t& flags, const std::string& text)
{
  return wex::test::get_path("test.h", flags).log().str().find(text) !=
         std::string::npos;
}

TEST_CASE("wex::path")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::path().empty());
    REQUIRE(wex::path("xxx").string() == "xxx");
    REQUIRE(wex::path(wex::path("yyy")).string() == "yyy");
    wex::path fn = wex::test::get_path("test.h");
    REQUIRE(wex::path(fn).filename() == "test.h");
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
    REQUIRE(path.filename() == "test.h");
    REQUIRE(!path.empty());
    REQUIRE(path.log().str().find("test.h") != std::string::npos);
    REQUIRE(path.name() == "test");
    REQUIRE(!path.parent_path().empty());
    REQUIRE(!path.paths().empty());
    REQUIRE(path.stat().is_ok());
    REQUIRE(!path.is_readonly());

    REQUIRE(
      path.append(wex::path("error")).string().find("error") !=
      std::string::npos);

    path.replace_filename("xxx");

    REQUIRE(!wex::path("XXXXX").stat().is_ok());

    REQUIRE(wex::path("XXXXX").make_absolute().filename() == "XXXXX");
    REQUIRE(wex::path("XXXXX").make_absolute().string() != "XXXXX");
  }

  SUBCASE("log")
  {
    REQUIRE(log_contains(wex::path::log_t(), "test.h"));
    REQUIRE(!log_contains(wex::path::log_t(), "Modified"));
    REQUIRE(!log_contains(wex::path::log_t(), "test/data/test.h"));
    REQUIRE(!log_contains(wex::path::log_t(), "Synchronized"));

    REQUIRE(log_contains(
      wex::path::log_t().set(wex::path::LOG_SYNC),
      "Synchronized"));
    REQUIRE(
      !log_contains(wex::path::log_t().set(wex::path::LOG_SYNC), "Modified"));
    REQUIRE(!log_contains(
      wex::path::log_t().set(wex::path::LOG_SYNC),
      "test/data/test.h"));

    REQUIRE(
      log_contains(wex::path::log_t().set(wex::path::LOG_MOD), "Modified"));
    REQUIRE(!log_contains(
      wex::path::log_t().set(wex::path::LOG_MOD),
      "Synchronized"));
    REQUIRE(!log_contains(
      wex::path::log_t().set(wex::path::LOG_MOD),
      "test/data/test.h"));

    REQUIRE(log_contains(
      wex::path::log_t().set(wex::path::LOG_PATH),
      "test/data/test.h"));
    REQUIRE(
      !log_contains(wex::path::log_t().set(wex::path::LOG_PATH), "Modified"));

    REQUIRE(log_contains(
      wex::path::log_t().set(wex::path::LOG_PATH).set(wex::path::LOG_MOD),
      "test/data/test.h"));
    REQUIRE(log_contains(
      wex::path::log_t().set(wex::path::LOG_PATH).set(wex::path::LOG_MOD),
      "Modified"));
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
    const int  max = 1000;
    const auto exfile(wex::test::get_path("test.h"));
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.stat().is_readonly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - ex_start);
    const auto file(wex::test::get_path("test.h"));
    const auto wx_start = std::chrono::system_clock::now();

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
