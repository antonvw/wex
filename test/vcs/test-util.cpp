////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/path.h>
#include <wex/vcs/vcs.h>

#include "../../../src/vcs/util.h"
#include "test.h"

TEST_CASE("wex::utils")
{
  SECTION("execute_grep")
  {
    get_stc()->SetFocus();
    get_stc()->set_text("HERE");
    get_stc()->SelectAll();

    wex::path file(wex::test::get_path("test.h"));
    file.make_absolute();
    wex::vcs vcs(std::vector<wex::path>{file});

    REQUIRE(wex::execute_grep("xx", vcs.toplevel()));
  }

  SECTION("expand_macro")
  {
    wex::process_data data;
    REQUIRE(!wex::expand_macro(data, nullptr));

    data.exe("%LINES");
    REQUIRE(!wex::expand_macro(data, nullptr));

    REQUIRE(wex::expand_macro(data, get_stc()));
    REQUIRE(!data.exe().contains("%LINES"));
  }

  SECTION("vcs_diff")
  {
    REQUIRE(!wex::vcs_diff(""));
    REQUIRE(!wex::vcs_diff("XXX"));
    REQUIRE(wex::vcs_diff("diff"));
  }
}
