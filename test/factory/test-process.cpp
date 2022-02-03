////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
    SUBCASE("no handler")
    {
      REQUIRE(!process.async_system(wex::process_data("bash")));
    }

    wxEvtHandler out;
    process.set_handler_out(&out);

    SUBCASE("exe")
    {
      REQUIRE(process.async_system(wex::process_data("bash")));
      REQUIRE(process.get_exe() == "bash");
      REQUIRE(process.is_running());
      REQUIRE(process.write("xx"));
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
      process.stop();
      process.async_sleep_for(std::chrono::milliseconds(10));
      REQUIRE(!process.write("xx"));
    }

    SUBCASE("invalid")
    {
      REQUIRE(process.async_system(wex::process_data("xxxx")));
      process.stop();
      REQUIRE(!process.is_running());
    }
  }
#endif

  SUBCASE("system")
  {
#ifndef __WXMSW__
    SUBCASE("invalid")
    {
      REQUIRE(process.system(wex::process_data("xxxx")) != 0);
      REQUIRE(process.get_stdout().empty());
      REQUIRE(!process.get_stderr().empty());
    }
#endif

#ifdef __UNIX__
    SUBCASE("stdin")
    {
      REQUIRE(process.system(wex::process_data("wc -c").stdin("xxxxxx")) == 0);
      CAPTURE(process.get_stdout());
      REQUIRE(process.get_stdout().find("6") != std::string::npos);
    }

    SUBCASE("start_dir")
    {
      wex::path cwd;

      REQUIRE(process.system(wex::process_data("ls -l").start_dir("/")) == 0);
      REQUIRE(!process.get_stdout().empty());
      REQUIRE(wxGetCwd().Contains("data"));
      wex::path::current(cwd.original());
    }
#endif
  }
}
