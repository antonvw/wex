////////////////////////////////////////////////////////////////////////////////
// Name:      test-debug.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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

TEST_CASE("wxExDebug", "[process]")
{
  wxExDebug dbg(GetFrame());
  
  wxExMenu menu;
  
  REQUIRE( dbg.AddMenu(&menu) > 0);
  REQUIRE( dbg.AddMenu(&menu, true) > 0);

  int item = menu.FindItem("break");
  REQUIRE( item != wxNOT_FOUND);
  REQUIRE( item > ID_EDIT_DEBUG_FIRST );
  REQUIRE( item < ID_EDIT_DEBUG_LAST);
  
  REQUIRE( !dbg.Execute(item));
  
  wxExSTC* stc = GetSTC();
  stc->SetText("#include <stdio.h>\n\nmain()\n{printf(\"hello world\");\n}\n");
  stc->GetFile().FileSave("example.cc");
  system("cc -g example.cc");
  
  wxExProcess process;
  dbg.SetDebugProcess(&process);
  REQUIRE( !dbg.Execute(item));
  
  process.Execute("gdb a.out");
  
  REQUIRE(!dbg.Execute(item));
  stc->GotoLine(1);
  REQUIRE( dbg.Execute(item, stc));
  REQUIRE( stc->MarkerGet(1) > 0);
}
