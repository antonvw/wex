////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/versioninfo.h>
#include <wex/version.h>
#include "test.h"

TEST_CASE("wex::version")
{
  REQUIRE(!wex::version_info().get().empty());
  REQUIRE(!wex::get_version_info().get().empty());
}
