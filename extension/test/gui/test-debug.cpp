////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/process.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::debug")
{
  // obtain a menu item id
  wex::menu menu;

  // start test
  wex::debug dbg(frame());

  REQUIRE( dbg.process() == nullptr);
  REQUIRE( dbg.breakpoints().empty());
  REQUIRE( dbg.marker_breakpoint().number() > 0);

  
  wex::stc* stc = get_stc();
  
  if (stc != nullptr)
  {
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
  }
  
  REQUIRE( dbg.add_menu(&menu) > 0);
  REQUIRE( dbg.add_menu(&menu, true) > 0);
  const int item = menu.FindItem("break");
  REQUIRE( item != wxNOT_FOUND);
  REQUIRE( item > wex::ID_EDIT_DEBUG_FIRST );
  REQUIRE( item < wex::ID_EDIT_DEBUG_LAST);

#ifndef __WXMSW__
  REQUIRE( dbg.execute("break"));
  REQUIRE( dbg.execute("break all breakpoints"));
  REQUIRE( dbg.execute("break", get_stc()));
#endif
  REQUIRE( dbg.breakpoints().empty()); // no file loaded
#ifndef __WXMSW__
  REQUIRE( dbg.process() != nullptr);
#endif

#ifndef __WXMSW__
  REQUIRE( dbg.execute(item - wex::ID_EDIT_DEBUG_FIRST));
#endif
  REQUIRE(!dbg.execute(item));
  
  dbg.process_stdin("test");
  dbg.process_stdout("test");

/*    
  wex::stc* stc = get_stc();
  stc->set_text("#include <stdio.h>\n\nmain()\n{printf(\"hello world\");\n}\n");
  stc->get_file().file_save("example.cc");
  system("cc -g example.cc");
  
  wex::process process;
  REQUIRE( !dbg.execute(item));
  
  process.execute("gdb a.out");
  
  REQUIRE(!dbg.execute(item));
  stc->GotoLine(1);
  REQUIRE( dbg.execute(item, stc));
  REQUIRE( stc->MarkerGet(1) > 0);
  */
}
