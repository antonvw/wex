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
    REQUIRE(process.std_out().empty());
    REQUIRE(process.std_err().empty());
    REQUIRE(!process.is_debug());
    REQUIRE(!process.is_running());
    REQUIRE(process.data().exe().empty());
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
      REQUIRE(process.data().exe() == "bash");
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
      process.set_handler_dbg(&out); // if directly after out: crash
    }
  }
#endif

  SUBCASE("system")
  {
#ifndef __WXMSW__
    SUBCASE("invalid")
    {
      REQUIRE(process.system(wex::process_data("xxxx")) != 0);
      REQUIRE(process.std_out().empty());
      REQUIRE(!process.std_err().empty());
    }
#endif

#ifdef __UNIX__
    SUBCASE("stdin")
    {
      REQUIRE(process.system(wex::process_data("wc -c").std_in("xxxxxx")) == 0);
      CAPTURE(process.std_out());
      REQUIRE(process.std_err().empty());
      REQUIRE(process.std_out().find("6") != std::string::npos);
    }

    SUBCASE("start_dir")
    {
      wex::path cwd;

      REQUIRE(process.system(wex::process_data("ls -l").start_dir("/")) == 0);
      REQUIRE(process.std_err().empty());
      REQUIRE(!process.std_out().empty());
      REQUIRE(wxGetCwd().Contains("data"));
      wex::path::current(cwd.original());
    }
#endif
  }
}
