////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/file.h>

import<chrono>;

TEST_CASE("wex::file")
{
  SUBCASE("basic")
  {
    REQUIRE(!wex::file("XXXXX").is_open());

    wex::file file(
      wex::test::get_path("test.h"),
      std::ios_base::in | std::ios_base::out);

    REQUIRE(!file.check_sync());
    REQUIRE(!file.is_written());
    REQUIRE(!file.is_contents_changed());
    REQUIRE(file.is_open());

    file.reset_contents_changed();

    REQUIRE(!file.file_save());
    REQUIRE(!file.file_save(wex::path("test-save")));
    REQUIRE(!file.path().stat().is_ok());
    // The fullpath should be normalized, test it.
    REQUIRE(file.path().string() != "./test.h");
    REQUIRE(!file.path().stat().is_readonly());
    REQUIRE(file.file_load(wex::test::get_path("test.bin")));
    REQUIRE(file.open(wex::test::get_path("test.bin")));
    REQUIRE(file.is_open());

    const std::string* buffer = file.read();
    REQUIRE(buffer->length() == 40);

    REQUIRE(file.file_new(wex::path("test-xxx")));
    REQUIRE(file.open(std::ios_base::out));
    REQUIRE(file.is_open());
    REQUIRE(file.write(std::string("OK")));
    REQUIRE(file.write(*buffer));

    wex::file create(wex::path("test-create"), std::ios_base::out);
    REQUIRE(create.is_open());
    REQUIRE(create.write(std::string("OK")));
  }

  SUBCASE("append")
  {
    wex::file f(
      wex::path("test-create"),
      std::ios_base::out | std::ios_base::app);
    REQUIRE(f.is_open());
    REQUIRE(f.write("extra text"));
    REQUIRE(f.is_written());
  }

  // file should be closed before remove (at least for windows)
  SUBCASE("remove")
  {
    REQUIRE(remove("test-create") == 0);
    REQUIRE(remove("test-save") != 0);
    REQUIRE(remove("test-xxx") == 0);
  }

  SUBCASE("timing")
  {
    const int  max      = 100;
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      wex::file file(wex::test::get_path("test.h"));

      REQUIRE(file.read()->length() > 0);
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - ex_start);

    CHECK_LT(ex_milli.count(), 5000);
  }
}
