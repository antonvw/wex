////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/process.h>
#include <wex/stc.h>
#include "test.h"

TEST_SUITE_BEGIN("wex::process");

TEST_CASE("wex::debug")
{
  wex::process process;
  wex::debug dbg(frame(), &process);
  wex::menu menu;
  auto* stc = get_stc();
  
  if (stc != nullptr)
  {
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
  }
  
  SUBCASE("constructor")
  {
    REQUIRE( wex::debug(frame()).process() == nullptr);
    REQUIRE( dbg.process() != nullptr);
    REQUIRE( dbg.breakpoints().empty());
  }
  
  SUBCASE("menu")
  {
    REQUIRE( dbg.add_menu(&menu) > 0);
    REQUIRE( dbg.add_menu(&menu, true) > 0);
    const int item = menu.FindItem("run");

    REQUIRE( item > wex::ID_EDIT_DEBUG_FIRST );
    REQUIRE( item < wex::ID_EDIT_DEBUG_LAST);

#ifndef __WXMSW__
    REQUIRE( dbg.execute(item - wex::ID_EDIT_DEBUG_FIRST));
#endif

#ifndef __WXGTK__    
    REQUIRE(!dbg.execute(item));
#endif

    process.stop();
  }
  
  SUBCASE("execute")
  {
#ifndef __WXMSW__
    REQUIRE( dbg.execute("break"));
    REQUIRE( dbg.execute("break all breakpoints"));
    REQUIRE( dbg.execute("break", get_stc()));
#endif
    REQUIRE( dbg.breakpoints().empty()); // no file loaded
#ifndef __WXMSW__
    REQUIRE( dbg.process() != nullptr);
#endif

    process.stop();
  }
  
  SUBCASE("run")
  {
    stc->set_text(
      "#include <stdio.h>\n\n"
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

    REQUIRE( dbg.print("i"));
    REQUIRE( dbg.toggle_breakpoint(1, stc));
    REQUIRE(!dbg.apply_breakpoints(stc));
  
    process.stop();
    
    wxMilliSleep(10);
      
#ifndef __WXGTK__    
    REQUIRE(!dbg.execute(item));
    REQUIRE(!dbg.execute(item, stc));
#endif
  }

  SUBCASE("remove")
  {
    REQUIRE( remove("a.out") == 0);
    system("rm -rf a.out.dSYM");
    REQUIRE( remove("example.cc") == 0);
  }
}

TEST_SUITE_END();
