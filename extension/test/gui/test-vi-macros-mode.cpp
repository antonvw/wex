////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-macros-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi-macros-mode.h>
#include <wx/extension/variable.h>
#include "test.h"

TEST_CASE("wex::vi_macros_mode")
{
  std::string expanded;
  wex::vi_macros_mode mode;
  
  REQUIRE(!mode.IsRecording());
  REQUIRE(!mode.Expand(nullptr, wex::variable("test"), expanded));
  REQUIRE( mode.Transition("x") == 0);
  REQUIRE( mode.String().empty());
}
