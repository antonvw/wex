////////////////////////////////////////////////////////////////////////////////
// Name:      test-stat.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stat.h>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExStat" ) 
{
  wxExStat stat(GetTestFile().GetFullPath());

  REQUIRE( stat.IsOk());
  REQUIRE(!stat.IsReadOnly());
  REQUIRE( stat.Sync(GetTestDir() + "test-base.link"));
  REQUIRE( stat.Sync());
  REQUIRE(!stat.GetModificationTime().empty());

#ifdef __UNIX__
  REQUIRE( wxExStat("/etc/hosts").IsReadOnly());
#endif
}
