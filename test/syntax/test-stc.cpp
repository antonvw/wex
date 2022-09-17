////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

TEST_CASE("wex::syntax::stc")
{
  auto* stc = new wex::test::stc();
  stc->set_text("more text\notherline\nother line");

  // CAPTURE(stc->lexer_name());
  // REQUIRE(stc->lexer_name().empty());

  //  REQUIRE(stc->get_lexer().set("cpp"));
}
