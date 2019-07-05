////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-macros-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/vi-macros-mode.h>
#include <wex/variable.h>
#include "test.h"

TEST_CASE("wex::vi_macros_mode")
{
  std::string expanded;
  wex::vi_macros_mode mode;
  
  REQUIRE(!mode.is_playback());
  REQUIRE(!mode.is_recording());
  REQUIRE(!mode.expand(nullptr, wex::variable("test"), expanded));
  REQUIRE( mode.transition("x") == 0);
  REQUIRE( mode.string().empty());
}
