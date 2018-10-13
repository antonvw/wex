////////////////////////////////////////////////////////////////////////////////
// Name:      test-stat.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stat.h>
#include "../test.h"

TEST_CASE( "wex::stat" ) 
{
  wex::stat stat(GetTestPath("test.h").Path().string());

  REQUIRE( stat.IsOk());
  REQUIRE(!stat.GetModificationTime().empty());
  REQUIRE(!stat.IsReadOnly());
  REQUIRE( stat.Sync(GetTestPath("test-base.link").Path().string()));
  REQUIRE( stat.Sync());
  REQUIRE(!stat.GetModificationTime().empty());

#ifdef __UNIX__
  REQUIRE( wex::stat("/etc/hosts").IsReadOnly());
#endif
}
