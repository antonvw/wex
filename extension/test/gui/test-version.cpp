////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/version.h>
#include "test.h"

TEST_CASE("wex::version")
{
  REQUIRE(!wex::version_info().Get().empty());
  REQUIRE(!wex::get_version_info().Get().empty());
}
