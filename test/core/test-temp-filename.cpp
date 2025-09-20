////////////////////////////////////////////////////////////////////////////////
// Name:      test-temp-filename.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/file.h>
#include <wex/core/temp-filename.h>
#include <wex/test/test.h>

TEST_CASE("wex::temp_filename")
{
  SECTION("constructor")
  {
    wex::temp_filename tmpx, tmpy;

    REQUIRE(!tmpx.name().empty());
    REQUIRE(!tmpy.name().empty());
    REQUIRE(tmpx.name() != tmpy.name());
  }

  SECTION("constructor-cleanup")
  {
    wex::temp_filename tmpx(true);
    wex::path          p;

    REQUIRE(!tmpx.name().empty());

    {
      wex::temp_filename tmpy(true);

      wex::path p = wex::path(tmpy.name());
      wex::file file(p);

      REQUIRE(file.write(std::string("xyz")));
      REQUIRE(p.file_exists());
    }

    REQUIRE(!p.file_exists());
  }
}
