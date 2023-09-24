////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>

#include "test.h"

TEST_CASE("wex::ex-mode")
{
  auto* stc = get_stc();
  auto* ex  = &stc->get_vi();
  ex->use(wex::ex::EX);
  stc->DocumentStart();

  SUBCASE("find")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());
    ex->ex_stream()->stream(ifs);

    REQUIRE(ex->visual() == wex::ex::EX);
    REQUIRE(ex->command(":/w/"));
    REQUIRE(ex->ex_stream()->get_current_line() == 2);
    REQUIRE(ex->command("://"));
    REQUIRE(ex->ex_stream()->get_current_line() == 9);
    REQUIRE(ex->command(":??"));
    REQUIRE(ex->ex_stream()->get_current_line() == 2);
  }

  ex->use(wex::ex::VISUAL);
}
