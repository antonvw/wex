////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-window.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/window-data.h>

TEST_CASE("wex::window_data")
{
  REQUIRE(wex::window_data().id() == wxID_ANY);
  REQUIRE(wex::window_data().name().empty());
  REQUIRE(wex::window_data().name("xxx").name() == "xxx");
}
