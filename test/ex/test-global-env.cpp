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
    wex::global_env ge(ex);

    REQUIRE(!ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 0);
  }

  SUBCASE("commands")
  {
    wex::addressrange::data().set_global("g/he/d");
    wex::global_env ge(ex);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);
  }

  SUBCASE("commands-append")
  {
    wex::addressrange::data().set_global("g/he/a|<XXX>");
    wex::global_env ge(ex);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);
  }

  SUBCASE("commands-change")
  {
    wex::addressrange::data().set_global("g/he/c|<XXX>");
    wex::global_env ge(ex);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);
  }

  SUBCASE("commands-insert")
  {
    wex::addressrange::data().set_global("g/he/i|<XXX>");
    wex::global_env ge(ex);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);
  }

  delete ex;
}
