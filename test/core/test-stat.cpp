////////////////////////////////////////////////////////////////////////////////
// Name:      test-stat.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/stat.h>

TEST_CASE("wex::file_stat")
{
  wex::file_stat stat(wex::test::get_path("test.h").string());

  REQUIRE(stat.is_ok());
  REQUIRE(stat.get_st_atime() != 0);
  REQUIRE(stat.get_st_ctime() != 0);
  REQUIRE(stat.get_st_mtime() != 0);
  REQUIRE(stat.get_st_size() != 0);
  REQUIRE(!stat.get_creation_time().empty());
  REQUIRE(!stat.get_modification_time().empty());
  REQUIRE(!stat.is_readonly());
  REQUIRE(stat.sync(wex::test::get_path("test-base.link").string()));
  REQUIRE(stat.sync());
  REQUIRE(!stat.get_creation_time().empty());
  REQUIRE(!stat.get_modification_time().empty());

#ifdef __UNIX__
  REQUIRE(wex::file_stat("/etc/hosts").is_readonly());
#endif
}
