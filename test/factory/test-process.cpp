////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <wex/factory/process.h>

#include "../test.h"

TEST_CASE("wex::factory::process")
{
  wex::factory::process process;

  SUBCASE("constructor")
  {
    REQUIRE(process.get_stdout().empty());
    REQUIRE(process.get_stderr().empty());
    REQUIRE(!process.is_debug());
    REQUIRE(!process.is_running());
    REQUIRE(process.get_exe().empty());
    REQUIRE(!process.write("xx"));
  }

#ifndef __WXMSW__
  SUBCASE("async_system")
  {
    SUBCASE("no handler") { REQUIRE(!process.async_system("bash")); }

    wxEvtHandler out;
    process.set_handler_out(&out);

    SUBCASE("exe")
    {
      REQUIRE(process.async_system("bash"));
      REQUIRE(process.get_exe() == "bash");
      REQUIRE(process.is_running());
      REQUIRE(process.write("xx"));
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
      process.stop();
      REQUIRE(!process.write("xx"));
    }

    SUBCASE("invalid")
    {
      REQUIRE(process.async_system("xxxx"));
      process.stop();
      REQUIRE(!process.is_running());
    }
  }
#endif

  SUBCASE("system")
  {
    wex::path cwd;

#ifdef __UNIX__
    SUBCASE("start_dir")
    {
      REQUIRE(process.system("ls -l", "/") == 0);
      REQUIRE(!process.get_stdout().empty());
      REQUIRE(wxGetCwd().Contains("data"));
      wex::path::current(cwd.original());
    }

    SUBCASE("input")
    {
      //      REQUIRE(process.system("wc", "./") == 0);
      //      freopen("newstdin", "w", stdin);
      //      fprintf(stdin, "1 2 3");
      //      fclose(stdin);
      //      std::cout << process.get_stdout() << "\n";
    }
#endif

#ifndef __WXMSW__
    SUBCASE("invalid")
    {
      REQUIRE(process.system("xxxx") != 0);
      REQUIRE(process.get_stdout().empty());
      REQUIRE(!process.get_stderr().empty());
    }
#endif
  }
}
