////////////////////////////////////////////////////////////////////////////////
// Name:      test-auto-complete.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/auto-complete.h>

#include "test.h"

TEST_CASE("wex::auto_complete")
{
  auto* stc = new wex::stc(wex::test::get_path("test.h"));
  stc->SetFocus();
  frame()->pane_add(stc);

  wex::auto_complete ac(stc);

  SUBCASE("constructor")
  {
    REQUIRE(ac.use());
    REQUIRE(ac.insert().empty());
    REQUIRE(ac.inserts().empty());
    REQUIRE(ac.variable("none").empty());
  }

  SUBCASE("clear")
  {
    REQUIRE(!ac.on_char('x'));
    REQUIRE(!ac.on_char('y'));
    REQUIRE(!ac.on_char('z'));
    REQUIRE(ac.insert() == "xyz");
    ac.clear();
    REQUIRE(ac.insert().empty());
    REQUIRE(ac.inserts().find("xyz") != ac.inserts().end());
  }

  SUBCASE("complete")
  {
    REQUIRE(!ac.complete(std::string()));
    REQUIRE(ac.complete("test_app"));
    REQUIRE(!ac.on_char(WXK_BACK));

    ac.on_char('x');
    REQUIRE(!ac.on_char(WXK_BACK));
    REQUIRE(!ac.on_char(WXK_BACK));
    ac.on_char('x');
    ac.on_char('y');
    ac.on_char('z');
    REQUIRE(ac.insert() == "xyz");

    ac.on_char(';');
    REQUIRE(ac.insert().empty());
    REQUIRE(ac.inserts().find("xyz") != ac.inserts().end());

    ac.on_char('a');
    ac.on_char('b');
    ac.on_char('c');
    REQUIRE(ac.insert() == "abc");
    ac.on_char(';');
    REQUIRE(ac.inserts().find("xyz") != ac.inserts().end());
    REQUIRE(ac.inserts().find("abc") != ac.inserts().end());
    REQUIRE(ac.inserts().size() == 2);
  }

  SUBCASE("stc")
  {
    REQUIRE(stc->get_fold_level() == 0);

    event(stc, 'O');
    event(stc, '{');
    event(stc, WXK_RETURN);
    event(stc, WXK_RETURN);
    REQUIRE(stc->get_fold_level() == 1);

    event(stc, 't');
    event(stc, WXK_RETURN);
    REQUIRE(stc->get_fold_level() == 1);

    event(stc, ' ');
    event(stc, 'x');
    event(stc, 'x');
    event(stc, '.');
    event(stc, WXK_RETURN);
    event(stc, ' ');
    event(stc, 'y');
    event(stc, 'y');
    event(stc, '.');
    event(stc, WXK_RETURN);
    REQUIRE(stc->get_text().contains("test_app xx.method_one"));
    REQUIRE(stc->auto_complete()->variable("xx") == "test_app");
    REQUIRE(stc->auto_complete()->variable("yy") == "test_app");

    event(stc, ';');
    event(stc, WXK_RETURN);
    event(stc, 'x');
    event(stc, 'x');
    event(stc, '.');
    event(stc, WXK_RETURN);

    event(stc, WXK_RETURN);
    event(stc, '}');
    event(stc, WXK_RETURN);
    event(stc, WXK_RETURN);
    event(stc, WXK_RETURN);

    // Outcommented, when running verbose, this is ok.
#ifdef INVEST
    REQUIRE(stc->get_fold_level() == 0);
#endif
  }

  SUBCASE("sync")
  {
    REQUIRE(ac.sync());
    REQUIRE(!ac.sync());

    event(stc, 'O');
    event(stc, '{');
    event(stc, WXK_RETURN);
    event(stc, WXK_RETURN);

    REQUIRE(ac.sync());
    REQUIRE(!ac.sync());
  }

  SUBCASE("use")
  {
    REQUIRE(ac.use());

    ac.use(false);
    REQUIRE(!ac.use());
    REQUIRE(!ac.on_char('x'));
  }
}
