////////////////////////////////////////////////////////////////////////////////
// Name:      test-file-status.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/file-status.h>

TEST_CASE("wex::file_status")
{
  wex::file_status stat(wex::test::get_path("test.h").string());

  REQUIRE(stat.is_ok());
  REQUIRE(stat.get_access_time() != 0);
  REQUIRE(stat.get_creation_time() != 0);
  REQUIRE(stat.get_modification_time() != 0);
  REQUIRE(stat.get_size() != 0);
  REQUIRE(!stat.get_creation_time_str().empty());
  REQUIRE(!stat.get_modification_time_str().empty());
  REQUIRE(!stat.is_readonly());
  REQUIRE(stat.sync(wex::test::get_path("test-base.link").string()));
  REQUIRE(stat.sync());
  REQUIRE(!stat.get_creation_time_str().empty());
  REQUIRE(!stat.get_modification_time_str().empty());

#ifdef __UNIX__
  REQUIRE(wex::file_status("/etc/hosts").is_readonly());
#endif
}
