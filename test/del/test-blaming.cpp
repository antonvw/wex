////////////////////////////////////////////////////////////////////////////////
// Name:      test-blaming.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

#include "../src/del/blaming.h"

#include "test.h"

TEST_CASE("wex::blaming")
{
  SECTION("constructor-no-offset")
  {
    wex::blaming bl(get_stc(), std::string());

    REQUIRE(bl.renamed().empty());
    REQUIRE(bl.revision().empty());
    REQUIRE(bl.vcs().entry().name().empty());

    REQUIRE(!bl.execute(wex::path("xxx")));
  }

  SECTION("constructor-offset")
  {
    REQUIRE(
      ((wex::frame*)del_frame())->open_file(wex::test::get_path("test.h")));

    wex::blaming bl(get_stc(), "1000");

    REQUIRE(!bl.execute(wex::path("xxx")));
  }
}
