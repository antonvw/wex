////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi-mode.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>
#include "test.h"

TEST_CASE("wex::vi_mode")
{
  wex::vi_mode mode(&GetSTC()->GetVi());
  
  REQUIRE(!mode.GetInsertCommands().empty());

  // normal
  REQUIRE( mode.Normal());
  std::string command("x");
  REQUIRE(!mode.Transition(command));
  command = "y";
  REQUIRE(!mode.Transition(command));
  REQUIRE( mode.String().empty());
  
  // insert
  command = "i";
  REQUIRE( mode.Transition(command));
  REQUIRE( mode.Insert());
  command = "i";
  REQUIRE(!mode.Transition(command));
  REQUIRE( mode.Insert());
  REQUIRE( mode.String() == "insert");
  REQUIRE( mode.Escape());
  REQUIRE( mode.Normal());

  command = "cc";
  REQUIRE( mode.Transition(command));
  REQUIRE( mode.Insert());
  REQUIRE( mode.Escape());
  REQUIRE( mode.Normal());
  
  GetSTC()->SetReadOnly(true);
  command = "i";
  REQUIRE( mode.Transition(command));
  REQUIRE( mode.Normal());
  GetSTC()->SetReadOnly(false);
  
  for (const auto& visual : std::vector<std::pair<std::string, wex::vi_modes>> {
    {"v",wex::vi_modes::VISUAL},
    {"V",wex::vi_modes::VISUAL_LINE},
    {"K",wex::vi_modes::VISUAL_RECT}})
  {
    std::string command(visual.first);
    REQUIRE( mode.Transition(command));
    REQUIRE( mode.Get() == visual.second);
    command = visual.first;
    REQUIRE( mode.Transition(command)); // ignore
    REQUIRE( mode.Get() == visual.second);
    REQUIRE( mode.Escape());
    REQUIRE( mode.Normal());
  }
}
