////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/version.h>
#include <wx/versioninfo.h>

TEST_CASE("wex::version")
{
  REQUIRE(!wex::version_info().get().empty());
  REQUIRE(!wex::get_version_info().get().empty());
}
