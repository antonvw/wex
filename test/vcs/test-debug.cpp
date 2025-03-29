////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/factory/defs.h>
#include <wex/ui/menu.h>
#include <wex/vcs/debug.h>
#include <wex/vcs/process.h>

#include "test.h"

TEST_CASE("wex::debug", "[!mayfail]")
{
  wex::config("debug.debugger").set(wex::debug::default_exe());

  wex::process process;
  wex::debug   dbg(frame(), &process);
  wex::menu    menu;
  auto*        stc = get_stc();

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();

  SECTION("constructor")
  {
    REQUIRE(!wex::debug(frame()).is_active());
    REQUIRE(dbg.breakpoints().empty());
  }

  SECTION("execute")
  {
#ifdef __WXOSX__
    REQUIRE(dbg.execute("detach"));
    REQUIRE(!dbg.execute("attach xxx"));
    REQUIRE(!dbg.execute("break"));
    REQUIRE(!dbg.execute("break all breakpoints"));
    REQUIRE(dbg.execute("break", get_stc()));
    REQUIRE(dbg.is_active());
#endif
    REQUIRE(dbg.breakpoints().empty()); // no file loaded

    process.stop();

    REQUIRE(!dbg.is_active());
  }

  SECTION("menu")
  {
    REQUIRE(dbg.add_menu(&menu) > 0);
    REQUIRE(dbg.add_menu(&menu, true) > 0);
    const int item = menu.FindItem("run");

    REQUIRE(item > wex::ID_EDIT_DEBUG_FIRST);
    REQUIRE(item < wex::ID_EDIT_DEBUG_LAST);

#ifndef __WXMSW__
#ifndef GITHUB
    REQUIRE(dbg.execute(item - wex::ID_EDIT_DEBUG_FIRST));
    REQUIRE(!dbg.execute(item));
#endif
#endif
  }

#ifndef __WXMSW__
  SECTION("run")
  {
    stc->set_text("#include <stdio.h>\n\n"
                  "int main()\n"
                  "{\n"
                  "  printf(\"hello world\");\n"
                  "}\n");

    stc->get_file().file_save(wex::path("example.cc"));
    system("cc -g example.cc");

    const int item = menu.FindItem("run");
#ifndef __WXGTK__
    REQUIRE(!dbg.execute(item));
#endif

    dbg.execute(wex::config("debug.debugger").get() + " a.out");

    REQUIRE(!dbg.debug_entry().name().empty());

    REQUIRE(dbg.is_active());
    REQUIRE(dbg.print("main"));
    REQUIRE(!dbg.toggle_breakpoint(1, nullptr));
    REQUIRE(dbg.toggle_breakpoint(1, stc));
    REQUIRE(!dbg.apply_breakpoints(stc));

    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wex::ID_DEBUG_STDIN);
    event.SetString("del 0");
    wxPostEvent(&dbg, event);

    process.stop();
    process.async_sleep_for(std::chrono::milliseconds(10));

    REQUIRE(!dbg.is_active());

#ifndef __WXGTK__
    REQUIRE(!dbg.execute(item));
    REQUIRE(!dbg.execute(item, stc));
#endif
  }
#endif

#ifndef __WXMSW__
  SECTION("remove")
  {
    REQUIRE(remove("a.out") == 0);
    system("rm -rf a.out.dSYM");
    REQUIRE(remove("example.cc") == 0);
  }
#endif

  SECTION("static")
  {
    REQUIRE(!wex::debug::default_exe().empty());
  }
}
