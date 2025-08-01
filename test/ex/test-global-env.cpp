////////////////////////////////////////////////////////////////////////////////
// Name:      test-global-env.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>

#include "../src/ex/global-env.h"
#include "test.h"

void test_global(const std::string& cmd, const wex::addressrange& ar)
{
  REQUIRE(wex::addressrange::data().set_global(cmd));
  wex::global_env ge(ar);

  REQUIRE(ge.has_commands());
  REQUIRE(ge.global(wex::addressrange::data()));
  REQUIRE(ge.hits() == 3);
}

TEST_CASE("wex::global_env")
{
  auto* stc = new wex::test::stc();

  const wex::path p("test.h");
  ALLOW_CALL(*stc, path()).RETURN(p);

  stc->set_text(
    "hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy\n\n");

  auto*             ex = new wex::ex(stc);
  wex::addressrange ar(ex, "%");

  SECTION("constructor")
  {
    REQUIRE(wex::addressrange::data().set_global("g/xx/"));
    wex::global_env ge(ar);

    REQUIRE(!ge.has_commands());
    REQUIRE(ge.hits() == 0);
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 0);
  }

  SECTION("commands")
  {
    REQUIRE(wex::addressrange::data().set_global("g/he/"));
    wex::global_env ge(ar);
    REQUIRE(!ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 3);

    REQUIRE(wex::addressrange::data().set_global("g/^$/"));
    wex::global_env el(ar); // empty line
    REQUIRE(!el.has_commands());
    REQUIRE(el.global(wex::addressrange::data()));
    REQUIRE(el.hits() == 2);

    REQUIRE(wex::addressrange::data().set_global("v/he/"));
    wex::global_env inv(ar);
    REQUIRE(!inv.has_commands());
    REQUIRE(inv.global(wex::addressrange::data()));
    REQUIRE(inv.hits() == 9);

    REQUIRE(wex::addressrange::data().set_global("g!/he/"));
    wex::global_env inv2(ar);
    REQUIRE(!inv2.has_commands());
    REQUIRE(inv2.global(wex::addressrange::data()));
    REQUIRE(inv2.hits() == 9);
  }

  SECTION("commands-2addr")
  {
    REQUIRE(wex::addressrange::data().set_global("g/he/d"));
    wex::addressrange ar(ex, "1,2");
    wex::global_env   ge(ar);

    REQUIRE(ge.has_commands());
    REQUIRE(ge.global(wex::addressrange::data()));
    REQUIRE(ge.hits() == 2);

    stc->set_text(
      "hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy\n\n");
    REQUIRE(wex::addressrange::data().set_global("g!/gcc/d"));
    wex::addressrange ar_inv(ex, "5,6");
    wex::global_env   inv(ar_inv);

    REQUIRE(inv.has_commands());
    REQUIRE(inv.global(wex::addressrange::data()));
    REQUIRE(inv.hits() == 2);
  }

  SECTION("commands-append")
  {
    test_global("g/he/a|added he <XXX>|a|other", ar);

    wex::log_none off;
    REQUIRE(wex::addressrange::data().set_global("g/he/a"));
    wex::global_env ge_error(ar);
    REQUIRE(!ge_error.has_commands());
    // now it acts as g/he/, fix for 25.04
    REQUIRE(ge_error.global(wex::addressrange::data()));
    REQUIRE(ge_error.hits() == 9);
  }

  SECTION("commands-change")
  {
    test_global("g/he/c|<XXX>", ar);

    // and check whether undo works
    REQUIRE(ex->get_stc()->get_text().contains("<XXX>"));
    ex->get_stc()->Undo();
    REQUIRE(!ex->get_stc()->get_text().contains("<XXX>"));
  }

  SECTION("commands-delete")
  {
    test_global("g/he/d", ar);
  }

  SECTION("commands-insert")
  {
    test_global("g/he/i|<XXX>", ar);
  }

  SECTION("commands-substitute")
  {
    test_global("g/he/s/ll/LL", ar);
  }

  delete ex;
}
