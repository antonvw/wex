////////////////////////////////////////////////////////////////////////////////
// Name:      test-global-env.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
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
    wex::addressrange::data().set_global("g/he/a|added he <XXX>|a|other");
    wex::global_env ge(ex);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);

    wex::log_none off;
    wex::addressrange::data().set_global("g/he/a");
    wex::global_env ge_error(ex);
    REQUIRE(!ge_error.has_commands());
    // now it acts as g/he/, fix for 25.04
    REQUIRE(ge_error.global(wex::addressrange::data()));
    REQUIRE(ge_error.hits() == 9);
  }

  SUBCASE("commands-change")
  {
    wex::addressrange::data().set_global("g/he/c|<XXX>");
    wex::global_env ge(ex);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);

    // and check whether undo works
    REQUIRE(ex->get_stc()->get_text().contains("<XXX>"));
    ex->get_stc()->Undo();
    REQUIRE(!ex->get_stc()->get_text().contains("<XXX>"));
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
