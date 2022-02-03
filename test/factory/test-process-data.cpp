////////////////////////////////////////////////////////////////////////////////
// Name:      test-process-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/process-data.h>

#include "../test.h"

TEST_CASE("wex::factory::process_data")
{
  SUBCASE("constructor-default")
  {
    wex::process_data data;

    REQUIRE(data.exe().empty());
    REQUIRE(data.start_dir().empty());
    REQUIRE(data.std_in().empty());
  }

  SUBCASE("constructor")
  {
    wex::process_data data("xx");

    REQUIRE(data.exe() == "xx");
    REQUIRE(data.start_dir().empty());
    REQUIRE(data.std_in().empty());
  }

  SUBCASE("log")
  {
    wex::process_data data("xx");

    REQUIRE(data.log() == "exe: xx");
  }

  SUBCASE("set")
  {
    wex::process_data data("xx");

    data.exe("yy").std_in("zz").start_dir("ww");

    REQUIRE(data.exe() == "yy");
    REQUIRE(data.start_dir() == "ww");
    REQUIRE(data.std_in() == "zz");
  }
}
