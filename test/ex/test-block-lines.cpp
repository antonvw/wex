////////////////////////////////////////////////////////////////////////////////
// Name:      test-block-lines.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex.h>

#include "../src/ex/block-lines.h"
#include "test.h"

TEST_CASE("wex::block_lines")
{
  auto* stc = get_stc();

  stc->set_text("hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy");

  SUBCASE("constructor")
  {
    wex::block_lines bl(stc, 2, 5);

    REQUIRE(bl.type() == wex::block_lines::block_t::MATCH);
    REQUIRE(bl.get_range() == "3,5");
    REQUIRE(bl.size() == 3);
  }

  SUBCASE("constructor-inverse")
  {
    wex::block_lines bl(stc, 0, 0, wex::block_lines::block_t::INVERSE);

    REQUIRE(bl.type() == wex::block_lines::block_t::INVERSE);
    REQUIRE(bl.get_range() == "1");
    REQUIRE(bl.size() == 1);
  }

  SUBCASE("actions")
  {
    wex::block_lines bl(stc, 0, 0, wex::block_lines::block_t::INVERSE);
    wex::block_lines normal(stc, 4, 8);
    REQUIRE(bl.get_range() == "1");
    REQUIRE(bl.is_available());

    bl.finish(normal);
    REQUIRE(bl.get_range() == "2,4");

    bl.start(12);
    bl.end(14);
    REQUIRE(bl.get_range() == "13,14");
    bl.log();

    bl.start(15);
    bl.end(14);
    REQUIRE(!bl.is_available());
    REQUIRE(bl.get_range() == "");
  }
}
