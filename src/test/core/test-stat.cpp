////////////////////////////////////////////////////////////////////////////////
// Name:      test-stat.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stat.h>
#include "../test.h"

TEST_CASE( "wex::file_stat" ) 
{
  wex::file_stat stat(wex::test::get_path("test.h").string());

  REQUIRE( stat.is_ok());
  REQUIRE(!stat.get_modification_time().empty());
  REQUIRE(!stat.is_readonly());
  REQUIRE( stat.sync(wex::test::get_path("test-base.link").string()));
  REQUIRE( stat.sync());
  REQUIRE(!stat.get_modification_time().empty());

#ifdef __UNIX__
  REQUIRE( wex::file_stat("/etc/hosts").is_readonly());
#endif
}
