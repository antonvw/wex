////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-macros-mode.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi-macros-mode.h>
#include "test.h"

TEST_CASE("wxExViMacrosMode")
{
  wxExViMacrosMode mode;
  
  REQUIRE(!mode.IsPlayback());
  REQUIRE(!mode.IsRecording());
  REQUIRE( mode.Transition("x") == 0);
  REQUIRE( mode.String().empty());
}
