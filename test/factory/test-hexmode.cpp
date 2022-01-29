////////////////////////////////////////////////////////////////////////////////
// Name:      test-hexmode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/hexmode.h>

#include "test.h"

TEST_CASE("wex::factory::hexmode")
{
  auto* stc = new wex::test::stc();

  SUBCASE("constructor")
  {
    wex::factory::hexmode hm(stc);
    REQUIRE(!hm.is_active());
    REQUIRE(hm.get_stc() == stc);
    REQUIRE(hm.bytes_per_line() > 0);
    REQUIRE(hm.each_hex_field() > 0);
  }

  SUBCASE("lines")
  {
    wex::factory::hexmode hm(stc);
    REQUIRE(hm.lines("main()").empty());

    hm.make_active(true);
    REQUIRE(
      hm.lines("main()") ==
      "6D 61 69 6E 28 29                               main()");
  }

  SUBCASE("make_active")
  {
    wex::factory::hexmode hm(stc);
    hm.make_active(true);
    REQUIRE(hm.is_active());
  }

  SUBCASE("printable")
  {
    wex::factory::hexmode hm(stc);
    REQUIRE(hm.printable('x') == 'x');
    REQUIRE(hm.printable('\n') == '\n');

    hm.make_active(true);
    REQUIRE(hm.printable('x') == 'x');
    REQUIRE(hm.printable('\n') == '.');
  }
}
