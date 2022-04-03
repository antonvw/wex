////////////////////////////////////////////////////////////////////////////////
// Name:      test-process-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/process-data.h>

#include "../test.h"

TEST_CASE("wex::process_data")
{
  SUBCASE("constructor-default")
  {
    wex::process_data data;

    REQUIRE(data.args().empty());
    REQUIRE(data.args_str().empty());
    REQUIRE(data.exe().empty());
    REQUIRE(data.exe_path().empty());
    REQUIRE(data.start_dir().empty());
    REQUIRE(data.std_in().empty());
  }

  SUBCASE("constructor")
  {
    wex::process_data data("wc yy");

    REQUIRE(data.args().size() == 1);
    REQUIRE(data.args_str().empty());
    REQUIRE(data.args().front() == "yy");
    REQUIRE(data.exe() == "wc yy");
#ifdef __UNIX__
    REQUIRE(data.exe_path() == "/usr/bin/wc");
    REQUIRE(data.log() == "exe: /usr/bin/wc args: yy");
#endif
    REQUIRE(data.start_dir().empty());
    REQUIRE(data.std_in().empty());
  }

  SUBCASE("constructor-args")
  {
    wex::process_data data("wc", "yy -l");

    REQUIRE(data.args().size() == 2);
    REQUIRE(data.args_str() == "yy -l");
  }

  SUBCASE("log")
  {
    wex::process_data data("xx -c -v");

    REQUIRE(data.log() == "exe: xx args: -c -v");
  }

  SUBCASE("set")
  {
    wex::process_data data("xx");

    data.exe("yy").std_in("zz").start_dir("ww").args("xx");

    REQUIRE(data.exe() == "yy");
    REQUIRE(data.args_str() == "xx");
    REQUIRE(data.start_dir() == "ww");
    REQUIRE(data.std_in() == "zz");
  }
}
