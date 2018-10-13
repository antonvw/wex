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
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::debug")
{
  // obtain a menu item id
  wex::menu menu;

  // start test
  wex::debug dbg(GetFrame());

  REQUIRE( dbg.GetProcess() == nullptr);
  REQUIRE( dbg.GetBreakpoints().empty());
  REQUIRE( dbg.GetMarkerBreakpoint().GetNo() > 0);

  
  wex::stc* stc = GetSTC();
  
  if (stc != nullptr)
  {
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
  }
  
  REQUIRE( dbg.AddMenu(&menu) > 0);
  REQUIRE( dbg.AddMenu(&menu, true) > 0);
  const int item = menu.FindItem("break");
  REQUIRE( item != wxNOT_FOUND);
  REQUIRE( item > wex::ID_EDIT_DEBUG_FIRST );
  REQUIRE( item < wex::ID_EDIT_DEBUG_LAST);

#ifndef __WXMSW__
  REQUIRE( dbg.Execute("break"));
  REQUIRE( dbg.Execute("break all breakpoints"));
  REQUIRE( dbg.Execute("break", GetSTC()));
#endif
  REQUIRE( dbg.GetBreakpoints().empty()); // no file loaded
#ifndef __WXMSW__
  REQUIRE( dbg.GetProcess() != nullptr);
#endif

#ifndef __WXMSW__
  REQUIRE( dbg.Execute(item - wex::ID_EDIT_DEBUG_FIRST));
#endif
  REQUIRE(!dbg.Execute(item));
  
  dbg.ProcessStdIn("test");
  dbg.ProcessStdOut("test");

/*    
  wex::stc* stc = GetSTC();
  stc->SetText("#include <stdio.h>\n\nmain()\n{printf(\"hello world\");\n}\n");
  stc->GetFile().FileSave("example.cc");
  system("cc -g example.cc");
  
  wex::process process;
  REQUIRE( !dbg.Execute(item));
  
  process.Execute("gdb a.out");
  
  REQUIRE(!dbg.Execute(item));
  stc->GotoLine(1);
  REQUIRE( dbg.Execute(item, stc));
  REQUIRE( stc->MarkerGet(1) > 0);
  */
}
