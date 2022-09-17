////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer-props.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/lexer-props.h>

#include "../test.h"

TEST_CASE("wex::lexer_props")
{
  const wex::lexer_props l;

  REQUIRE(l.make_comment("").find(';') == std::string::npos);
  REQUIRE(l.make_comment("test").find(';') != std::string::npos);

  REQUIRE(l.make_key("xx", "yy").find('=') != std::string::npos);
  REQUIRE(l.make_key("xx", "yy", "zz").find('=') != std::string::npos);
  REQUIRE(l.make_key("xx", "yy", "zz").find(';') != std::string::npos);

  REQUIRE(l.make_section("test").find('[') != std::string::npos);
}
