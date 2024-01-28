////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/stc/shell.h>
#include <wex/vcs/process.h>

#include "test.h"

void check_process(wex::process& process, const std::string& text)
{
  REQUIRE(process.system(wex::process_data("echo %LINES")) == 0);
  CAPTURE(text);
  CAPTURE(process.std_out());
  REQUIRE(!process.std_out().contains("%LINES"));
  REQUIRE(process.std_out().contains(text));
}

TEST_CASE("wex::process")
{
  wex::path cwd;

#ifndef GITHUB
  SUBCASE("static")
  {
    REQUIRE(wex::process::prepare_output(frame()) != nullptr);
    REQUIRE(wex::process::get_shell() != nullptr);

    frame()->pane_add(wex::process::get_shell());
    wex::process::get_shell()->set_text(std::string());
  }
#endif

  wex::process process;

  SUBCASE("constructor")
  {
    REQUIRE(process.get_frame() != nullptr);
  }

#ifdef __UNIX__
  SUBCASE("async_system")
  {
    SUBCASE("exe")
    {
      REQUIRE(process.async_system(wex::process_data("bash")));
      REQUIRE(process.is_running());
      REQUIRE(!process.data().exe().empty());
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
    }

#ifndef GITHUB
    SUBCASE("invalid")
    {
      wex::log_none off;
      REQUIRE(process.async_system(wex::process_data("xxxx")));
      process.async_sleep_for(std::chrono::milliseconds(25));
      REQUIRE(!process.is_running());
    }
#endif

    SUBCASE("macro")
    {
      REQUIRE(process.async_system(wex::process_data("echo %LINES")));
    }
  }
#endif

  SUBCASE("dialog")
  {
    wex::process::config_dialog(wex::data::window().button(wxAPPLY | wxCANCEL));
  }

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
      REQUIRE(!process.data().exe().empty());
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
      wex::log_none off;
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

TEST_CASE("wex::process-macro" * doctest::skip())
{
  wex::process process;

  if (process.get_shell() != nullptr)
  {
    REQUIRE(process.system(wex::process_data("ls -l")) == 0);
    process.show_output();
    process.get_shell()->SetFocus();
    process.get_shell()->DocumentEnd();

    // only success if run as separate case?
    // ./test/vcs/wex-test-vcs -tc=wex::process-macro -ns
    check_process(process, "15,15");

    process.get_shell()->SetSelection(1, 5);
    check_process(process, "1,1");
  }
}
