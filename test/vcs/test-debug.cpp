////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/factory/defs.h>
#include <wex/ui/menu.h>
#include <wex/vcs/debug.h>
#include <wex/vcs/process.h>

#include "test.h"

TEST_CASE("wex::debug" * doctest::may_fail())
{
#ifdef __WXOSX__
  wex::config("debug.debugger").set("lldb");
#endif

  wex::process process;
  wex::debug   dbg(frame(), &process);
  wex::menu    menu;
  auto*        stc = get_stc();

  if (stc != nullptr)
  {
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
  }

  SUBCASE("constructor")
  {
    REQUIRE(!wex::debug(frame()).is_active());
    REQUIRE(dbg.breakpoints().empty());
  }

  SUBCASE("execute")
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

  SUBCASE("menu")
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
  SUBCASE("run")
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

#ifndef __WXOSX__
    dbg.execute("gdb a.out");
#else
    dbg.execute("lldb a.out");
#endif

    REQUIRE(!dbg.debug_entry().name().empty());

    REQUIRE(dbg.is_active());
    REQUIRE(dbg.print("main"));
    REQUIRE(!dbg.toggle_breakpoint(1, nullptr));
    REQUIRE(dbg.toggle_breakpoint(1, stc));
    REQUIRE(!dbg.apply_breakpoints(stc));

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
  SUBCASE("remove")
  {
    REQUIRE(remove("a.out") == 0);
    system("rm -rf a.out.dSYM");
    REQUIRE(remove("example.cc") == 0);
  }
#endif
}
