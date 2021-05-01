////////////////////////////////////////////////////////////////////////////////
// Name:      test-auto_complete.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/auto-complete.h>

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
  }

  SUBCASE("clear")
  {
    REQUIRE(ac.on_char('x'));
    REQUIRE(ac.on_char('y'));
    REQUIRE(ac.on_char('z'));
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

    REQUIRE(ac.on_char('x'));
    REQUIRE(ac.on_char(WXK_BACK));
    REQUIRE(!ac.on_char(WXK_BACK));

    REQUIRE(ac.on_char('x'));
    REQUIRE(ac.on_char('y'));
    REQUIRE(ac.on_char('z'));
    REQUIRE(ac.insert() == "xyz");
    REQUIRE(!ac.on_char(';'));
    REQUIRE(ac.insert().empty());
    REQUIRE(ac.inserts().find("xyz") != ac.inserts().end());

    REQUIRE(ac.on_char('a'));
    REQUIRE(ac.on_char('b'));
    REQUIRE(ac.on_char('c'));
    REQUIRE(ac.insert() == "abc");
    REQUIRE(!ac.on_char(';'));
    REQUIRE(ac.inserts().find("xyz") != ac.inserts().end());
    REQUIRE(ac.inserts().find("abc") != ac.inserts().end());
    REQUIRE(ac.inserts().size() == 2);
  }

  SUBCASE("level")
  {
    REQUIRE(ac.current_level() == 0);
    
    ac.on_char(' ');
    REQUIRE(ac.current_level() == 0);
    
    ac.on_char('{');
    ac.on_char('\n');
    REQUIRE(ac.current_level() == 0);
  }
  
  SUBCASE("use")
  {
    REQUIRE(ac.use());

    ac.use(false);
    REQUIRE(!ac.use());
    REQUIRE(!ac.on_char('x'));
  }
}
