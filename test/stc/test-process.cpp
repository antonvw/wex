////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/process.h>
#include <wex/stc/shell.h>

#include "test.h"

TEST_CASE("wex::process")
{
  wex::path cwd;

  SUBCASE("static")
  {
    REQUIRE(wex::process::prepare_output(frame()) != nullptr);
    REQUIRE(wex::process::get_shell() != nullptr);

    frame()->pane_add(wex::process::get_shell());
    wex::process::get_shell()->set_text(std::string());
  }

  wex::process process;

  SUBCASE("constructor") { REQUIRE(process.get_frame() != nullptr); }

  SUBCASE("dialog")
  {
    wex::process::config_dialog(wex::data::window().button(wxAPPLY | wxCANCEL));
  }

#ifdef __UNIX__
  SUBCASE("async_system")
  {
    SUBCASE("exe")
    {
      REQUIRE(process.async_system(wex::process_data("bash")));
      REQUIRE(process.is_running());
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
    }

    SUBCASE("invalid")
    {
      REQUIRE(process.async_system(wex::process_data("xxxx")));
      process.async_sleep_for(std::chrono::milliseconds(2500));
      REQUIRE(!process.is_running());
    }
  }
#endif

#ifdef __UNIX__
  SUBCASE("system")
  {
    SUBCASE("exe")
    {
      REQUIRE(process.system(wex::process_data("ls -l")) == 0);
      REQUIRE(!process.write("hello world"));
      REQUIRE(!process.std_out().empty());
      REQUIRE(process.std_err().empty());
      REQUIRE(!process.is_running());
      REQUIRE(process.data().exe().empty());
      process.show_output();
    }

    SUBCASE("repeat")
    {
      REQUIRE(process.system(wex::process_data("ls -l")) == 0);
      REQUIRE(!process.is_running());
      REQUIRE(!process.std_out().empty());
    }

    SUBCASE("working directory")
    {
      REQUIRE(process.system(wex::process_data("ls -l").start_dir("/")) == 0);
      REQUIRE(!process.std_out().empty());
      REQUIRE(wxGetCwd().Contains("data"));
    }

#ifndef __WXGTK__
    SUBCASE("invalid")
    {
      REQUIRE(process.system(wex::process_data("xxxx")) != 0);
      REQUIRE(!process.is_running());
      REQUIRE(!process.std_err().empty());
      REQUIRE(process.std_out().empty());
    }
#endif

    SUBCASE("working directory")
    {
      REQUIRE(process.system(wex::process_data("ls -l").start_dir("/")) == 0);
      wex::path::current(cwd.original());
    }
  }
#endif
}
