////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer-props.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/lexer-props.h>

#include "../test.h"

TEST_CASE("wex::lexer_props")
{
  const wex::lexer_props l;

  REQUIRE(!l.make_comment("").contains(';'));
  REQUIRE(l.make_comment("test").contains(';'));

  REQUIRE(l.make_key("xx", "yy").contains('='));
  REQUIRE(l.make_key("xx", "yy", "zz").contains('='));
  REQUIRE(l.make_key("xx", "yy", "zz").contains(';'));

  REQUIRE(l.make_section("test").contains('['));
}
