////////////////////////////////////////////////////////////////////////////////
// Name:      test-window-data.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/window-data.h>
#include "test.h"

TEST_CASE("wex::window_data")
{
  REQUIRE( wex::window_data().Id() == wxID_ANY);
  REQUIRE( wex::window_data().Name().empty());
  REQUIRE( wex::window_data().Name("xxx").Name() == "xxx");
}
