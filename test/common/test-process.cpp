////////////////////////////////////////////////////////////////////////////////
// Name:      common/test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/managed-frame.h>
#include <wex/process.h>
#include <wex/shell.h>

TEST_SUITE_BEGIN("wex::process");

TEST_CASE("wex::process")
{
  wex::path cwd;

  SUBCASE("static")
  {
    REQUIRE(wex::process::prepare_output(frame()) != nullptr);
    REQUIRE(wex::process::get_shell() != nullptr);

    wex::test::add_pane(frame(), wex::process::get_shell());
    wex::process::get_shell()->set_text(std::string());
  }

  wex::process process;

  SUBCASE("constructor") { REQUIRE(process.get_frame() != nullptr); }

  SUBCASE("dialog")
  {
    process.config_dialog(wex::data::window().button(wxAPPLY | wxCANCEL));
  }

  SUBCASE("async")
  {
    SUBCASE("exe")
    {
      REQUIRE(process.execute("bash"));
      REQUIRE(process.is_running());
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
    }

    SUBCASE("invalid")
    {
      REQUIRE(process.execute("xxxx"));
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
    }
  }

#ifdef __UNIX__
  SUBCASE("system")
  {
    SUBCASE("exe")
    {
      REQUIRE(process.execute("ls -l", wex::process::EXEC_WAIT));
      REQUIRE(!process.write("hello world"));
      REQUIRE(!process.get_stdout().empty());
      REQUIRE(!process.is_running());
      REQUIRE(!process.get_exe().empty());
      process.show_output();
    }

    SUBCASE("repeat")
    {
      REQUIRE(process.execute("ls -l", wex::process::EXEC_WAIT));
      REQUIRE(!process.is_running());
      REQUIRE(!process.get_stdout().empty());
    }

    SUBCASE("working directory")
    {
      REQUIRE(process.execute("ls -l", wex::process::EXEC_WAIT, "/"));
      REQUIRE(!process.get_stdout().empty());
      REQUIRE(wxGetCwd().Contains("data"));
    }

#ifndef __WXGTK__
    SUBCASE("invalid")
    {
      REQUIRE(!process.execute("xxxx", wex::process::EXEC_WAIT));
      REQUIRE(!process.is_running());
      REQUIRE(!process.get_stderr().empty());
      REQUIRE(process.get_stdout().empty());
    }
#endif

    SUBCASE("working directory")
    {
      REQUIRE(process.execute("ls -l", wex::process::EXEC_WAIT, "/"));
      wex::path::current(cwd.original());
    }
  }
#endif
}

TEST_SUITE_END();
