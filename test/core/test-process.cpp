////////////////////////////////////////////////////////////////////////////////
// Name:      core/test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/process-core.h>

TEST_SUITE_BEGIN("wex::process");

TEST_CASE("wex::core::process")
{
  wex::core::process process;

  SUBCASE("constructor")
  {
    REQUIRE(process.get_stdout().empty());
    REQUIRE(process.get_stderr().empty());
    REQUIRE(!process.is_debug());
    REQUIRE(!process.is_running());
    REQUIRE(process.get_exe().empty());
    REQUIRE(!process.write("xx"));
  }

  SUBCASE("async")
  {
    SUBCASE("no handler") { REQUIRE(!process.async("bash")); }

    wxEvtHandler out;
    process.set_handler_out(&out);

    SUBCASE("exe")
    {
      REQUIRE(process.async("bash"));
      REQUIRE(process.get_exe() == "bash");
      REQUIRE(process.is_running());
      REQUIRE(process.write("xx"));
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
      REQUIRE(!process.stop());
      REQUIRE(!process.write("xx"));
    }

    SUBCASE("invalid")
    {
      REQUIRE(process.async("xxxx"));
      REQUIRE(process.stop());
      REQUIRE(!process.is_running());
    }
  }

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
#endif

#ifndef __WXGTK__
    SUBCASE("invalid")
    {
      REQUIRE(process.system("xxxx") != 0);
      REQUIRE(process.get_stdout().empty());
      REQUIRE(!process.get_stderr().empty());
    }
#endif
  }
}

TEST_SUITE_END();
