////////////////////////////////////////////////////////////////////////////////
// Name:      test-path.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/path-lexer.h>
#include <wex/test/test.h>

TEST_CASE("wex::path_lexer")
{
  wex::path_lexer p(wex::test::get_path("test.h"));

  REQUIRE(p.lexer().scintilla_lexer() == "cpp");
}
