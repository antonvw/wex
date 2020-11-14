////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/managed-frame.h>
#include <wex/menu.h>
#include <wex/process.h>
#include <wex/stc.h>

TEST_SUITE_BEGIN("wex::process");

TEST_CASE("wex::debug")
{
  wex::process process;
  wex::debug   dbg(frame(), &process);
  wex::menu    menu;
  auto*        stc = get_stc();

  if (stc != nullptr)
  {
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
  }

#ifdef __WXOSX__
  wex::config("debug.debugger").set("lldb");
#endif

  SUBCASE("constructor")
  {
    REQUIRE(!wex::debug(frame()).is_active());
    REQUIRE(dbg.breakpoints().empty());
  }

  SUBCASE("menu")
  {
    REQUIRE(dbg.add_menu(&menu) > 0);
    REQUIRE(dbg.add_menu(&menu, true) > 0);
    const int item = menu.FindItem("run");

    REQUIRE(item > wex::ID_EDIT_DEBUG_FIRST);
    REQUIRE(item < wex::ID_EDIT_DEBUG_LAST);

#ifndef __WXMSW__
    REQUIRE(dbg.execute(item - wex::ID_EDIT_DEBUG_FIRST));
    REQUIRE(!dbg.execute(item));
#endif
  }

  SUBCASE("execute")
  {
#ifndef __WXMSW__
    REQUIRE(dbg.execute("break"));
    REQUIRE(dbg.execute("break all breakpoints"));
    REQUIRE(dbg.execute("break", get_stc()));
    REQUIRE(dbg.is_active());
#endif
    REQUIRE(dbg.breakpoints().empty()); // no file loaded
#ifndef __WXMSW__
#endif

    process.stop();

    REQUIRE(!dbg.is_active());
  }

#ifndef __WXMSW__
  SUBCASE("run")
  {
    stc->set_text("#include <stdio.h>\n\n"
                  "int main()\n"
                  "{\n"
                  "  printf(\"hello world\");\n"
                  "}\n");

    stc->get_file().file_save("example.cc");
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

    REQUIRE(dbg.print("i"));
    REQUIRE(dbg.is_active());
    REQUIRE(dbg.toggle_breakpoint(1, stc));
    REQUIRE(!dbg.apply_breakpoints(stc));

    process.stop();

    wxMilliSleep(10);

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

TEST_SUITE_END();
