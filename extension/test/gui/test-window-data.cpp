////////////////////////////////////////////////////////////////////////////////
// Name:      test-window-data.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/window-data.h>
#include "test.h"

TEST_CASE("wxExWindowData")
{
  REQUIRE( wxExWindowData().Id() == wxID_ANY);
  REQUIRE( wxExWindowData().Name().empty());
  REQUIRE( wxExWindowData().Name("xxx").Name() == "xxx");
}
