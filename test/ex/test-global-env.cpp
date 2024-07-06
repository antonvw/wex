////////////////////////////////////////////////////////////////////////////////
// Name:      test-global-env.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>

#include "../src/ex/global-env.h"
#include "test.h"

TEST_CASE("wex::global_env")
{
  auto* stc = get_stc();

  stc->set_text("hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy");

  auto* ex = new wex::ex(stc);

  SUBCASE("constructor")
  {
    wex::addressrange::data().set_global("g/xx/");
    wex::addressrange ar(ex, "1,2");
    wex::global_env   ge(ar);

    REQUIRE(!ge.has_commands());
    REQUIRE(ge.hits() == 0);
  }

  SUBCASE("commands")
  {
    wex::addressrange::data().set_global("g/he/d");
    wex::addressrange ar(ex, "1,2");
    wex::global_env   ge(ar);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);
  }

  delete ex;
}
