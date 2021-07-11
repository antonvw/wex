////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/dir.h>

#include "test.h"

TEST_CASE("wex::data::dir")
{
  SUBCASE("constructor")
  {
    wex::data::dir dir;

    REQUIRE(dir.dir_spec().empty());
    REQUIRE(dir.file_spec().empty());
    REQUIRE(dir.find_replace_data() == nullptr);
    REQUIRE(dir.max_matches() == -1);
    REQUIRE(dir.type().test(wex::data::dir::FILES));
  }

  SUBCASE("type")
  {
    REQUIRE(wex::data::dir::type_t_def().test(wex::data::dir::FILES));
    REQUIRE(wex::data::dir::type_t_def().test(wex::data::dir::DIRS));
    REQUIRE(!wex::data::dir::type_t_def().test(wex::data::dir::HIDDEN));
    REQUIRE(wex::data::dir::type_t_def().test(wex::data::dir::RECURSIVE));
  }
}
