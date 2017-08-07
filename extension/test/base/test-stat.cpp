////////////////////////////////////////////////////////////////////////////////
// Name:      test-stat.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stat.h>
#include "../test.h"

TEST_CASE( "wxExStat" ) 
{
  wxExStat stat(GetTestFile().Path().string());

  REQUIRE( stat.IsOk());
  REQUIRE(!stat.GetModificationTime().empty());
  REQUIRE(!stat.IsReadOnly());
  REQUIRE( stat.Sync(GetTestDir() + "test-base.link"));
  REQUIRE( stat.Sync());
  REQUIRE(!stat.GetModificationTime().empty());

#ifdef __UNIX__
  REQUIRE( wxExStat("/etc/hosts").IsReadOnly());
#endif
}
