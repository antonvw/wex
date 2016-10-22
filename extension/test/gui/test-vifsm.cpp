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
  wxExViFSM fsm(&GetSTC()->GetVi(), 
    [=](const std::string& command){;},
    [=](const std::string& command){;});
  
  REQUIRE(!fsm.GetInsertCommands().empty());

  // normal
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  REQUIRE(!fsm.Transition("x"));
  REQUIRE(!fsm.Transition("y"));
  REQUIRE( fsm.StateString().empty());
  
  // insert
  REQUIRE( fsm.Transition("i"));
  REQUIRE( fsm.State() == wxExVi::MODE_INSERT);
  REQUIRE(!fsm.Transition("i"));
  REQUIRE( fsm.State() == wxExVi::MODE_INSERT);
  REQUIRE( fsm.StateString() == "insert");
  REQUIRE( fsm.Transition(ESC));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);

  REQUIRE( fsm.Transition("cc"));
  REQUIRE( fsm.State() == wxExVi::MODE_INSERT);
  REQUIRE( fsm.Transition(ESC));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  
  GetSTC()->SetReadOnly(true);
  REQUIRE( fsm.Transition("i"));
  REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  GetSTC()->SetReadOnly(false);
  
  for (const auto& visual : std::vector<std::pair<std::string, int>> {
    {"v",wxExVi::MODE_VISUAL},
    {"V",wxExVi::MODE_VISUAL_LINE},
    {"K",wxExVi::MODE_VISUAL_RECT}})
  {
    REQUIRE( fsm.Transition(visual.first));
    REQUIRE( fsm.State() == visual.second);
    REQUIRE( fsm.Transition(visual.first)); // ignore
    REQUIRE( fsm.State() == visual.second);
    REQUIRE( fsm.Transition(ESC));
    REQUIRE( fsm.State() == wxExVi::MODE_NORMAL);
  }

  wxExViFSMEntry entry(0, 1, 2, [=](const std::string& command){;});
  REQUIRE( entry.State() == 0);
  REQUIRE( entry.Action() == 1);
  REQUIRE( entry.Next("test") == 2);
}
