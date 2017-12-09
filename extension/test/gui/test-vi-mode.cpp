////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-mode.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
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

TEST_CASE("wxExViMode")
{
  wxExViMode mode(&GetSTC()->GetVi());
  
  REQUIRE(!mode.GetInsertCommands().empty());

  // normal
  REQUIRE( mode.Normal());
  REQUIRE(!mode.Transition("x"));
  REQUIRE(!mode.Transition("y"));
  REQUIRE( mode.String().empty());
  
  // insert
  REQUIRE( mode.Transition("i"));
  REQUIRE( mode.Insert());
  REQUIRE(!mode.Transition("i"));
  REQUIRE( mode.Insert());
  REQUIRE( mode.String() == "insert");
  REQUIRE( mode.Escape());
  REQUIRE( mode.Normal());

  REQUIRE( mode.Transition("cc"));
  REQUIRE( mode.Insert());
  REQUIRE( mode.Escape());
  REQUIRE( mode.Normal());
  
  GetSTC()->SetReadOnly(true);
  REQUIRE( mode.Transition("i"));
  REQUIRE( mode.Normal());
  GetSTC()->SetReadOnly(false);
  
  for (const auto& visual : std::vector<std::pair<std::string, wxExViModes>> {
    {"v",wxExViModes::VISUAL},
    {"V",wxExViModes::VISUAL_LINE},
    {"K",wxExViModes::VISUAL_RECT}})
  {
    REQUIRE( mode.Transition(visual.first));
    REQUIRE( mode.Get() == visual.second);
    REQUIRE( mode.Transition(visual.first)); // ignore
    REQUIRE( mode.Get() == visual.second);
    REQUIRE( mode.Escape());
    REQUIRE( mode.Normal());
  }
}
