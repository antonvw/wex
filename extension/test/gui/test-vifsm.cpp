////////////////////////////////////////////////////////////////////////////////
// Name:      test-vifsm.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vifsm.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>
#include "test.h"

#define ESC "\x1b"

TEST_CASE("wxExViFSM", "[stc][vi]")
{
  wxExSTC* stc = new wxExSTC(GetFrame());
  AddPane(GetFrame(), stc);
  
  wxExViFSM fsm(&stc->GetVi(), 
    [=](const std::string& command){;},
    [=](const std::string& command){;});
  
  REQUIRE(!fsm.GetInsertCommands().empty());
  
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  REQUIRE(!fsm.Transition("x"));
  REQUIRE(!fsm.Transition("y"));
  
  REQUIRE( fsm.Transition("i"));
  REQUIRE( fsm.State() == wxExVi::MODE_INSERT);
  REQUIRE(!fsm.Transition("i"));
  REQUIRE( fsm.State() == wxExVi::MODE_INSERT);
  REQUIRE( fsm.StateString() == "insert");
  
  REQUIRE( fsm.Transition(ESC));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  REQUIRE( fsm.StateString().empty());
  REQUIRE( fsm.Transition("K"));
  REQUIRE( fsm.State() == wxExVi::MODE_VISUAL_RECT);
  REQUIRE( fsm.Transition("K")); // ignore
  REQUIRE( fsm.State() == wxExVi::MODE_VISUAL_RECT);
  
  REQUIRE( fsm.Transition(ESC));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  REQUIRE( fsm.StateString().empty());
  
  REQUIRE( fsm.Transition("cc"));
  REQUIRE( fsm.State() == wxExVi::MODE_INSERT);
  
  REQUIRE( fsm.Transition(ESC));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  
  stc->SetReadOnly(true);
  REQUIRE( fsm.Transition("i"));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  
  wxExViFSMEntry entry(0, 1, 2, [=](const std::string& command){;});
  REQUIRE( entry.State() == 0);
  REQUIRE( entry.Action() == 1);
  REQUIRE( entry.Next("test") == 2);
}
