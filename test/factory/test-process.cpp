////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/factory/process.h>

#include <wex/test/test.h>

TEST_CASE("wex::factory::process")
{
  wex::factory::process process;

  SECTION("constructor")
  {
    REQUIRE(process.std_out().empty());
    REQUIRE(process.std_err().empty());
    REQUIRE(!process.is_debug());
    REQUIRE(!process.is_running());
    REQUIRE(process.data().exe().empty());
    REQUIRE(!process.write("xx"));
  }

#ifndef __WXMSW__
  SECTION("async_system")
  {
    SECTION("no handler")
    {
      REQUIRE(!process.async_system(wex::process_data("bash")));
    }

    wxEvtHandler out;
    process.set_handler_out(&out);

    SECTION("exe")
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
      process.set_handler_dbg(&out); // if directly after out: crash
    }

    SECTION("invalid")
    {
      wex::log_none off;
      REQUIRE(process.async_system(wex::process_data("xxxx")));
      process.set_handler_out(nullptr);
      process.stop();
      REQUIRE(!process.is_running());
    }
  }
#endif

  SECTION("system")
  {
#ifndef GITHUB
    SECTION("invalid")
    {
      wex::log_none off;
      REQUIRE(process.system(wex::process_data("xxxx")) != 0);
      REQUIRE(process.std_out().empty());
      REQUIRE(!process.std_err().empty());
    }
#endif

#ifndef GITHUB
#ifndef __WXMSW__
    SECTION("stdin")
    {
      REQUIRE(process.system(wex::process_data("wc -c").std_in("xxxxxx")) == 0);
      CAPTURE(process.std_out());
      REQUIRE(process.std_err().empty());
      REQUIRE(process.std_out().contains("6"));
    }

    SECTION("start_dir")
    {
      wex::path cwd;

      REQUIRE(process.system(wex::process_data("ls -l").start_dir("/")) == 0);
      REQUIRE(process.std_err().empty());
      REQUIRE(!process.std_out().empty());
      REQUIRE(wxGetCwd().Contains("data"));
      wex::path::current(cwd.original());
    }
#endif
#endif
  }
}
