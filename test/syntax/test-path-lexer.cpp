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
  SUBCASE("default-constructor")
  {
    wex::path_lexer p("build.ninja");

    REQUIRE(p.lexer().display_lexer() == "ninja");
    REQUIRE(p.is_build());
  }

  SUBCASE("constructor-path")
  {
    wex::path_lexer p(wex::test::get_path("test.h"));

    REQUIRE(p.lexer().scintilla_lexer() == "cpp");
    REQUIRE(!p.is_build());
  }
}

TEST_CASE("wex::build")
{
#ifdef __UNIX__
  wex::path cwd; // as /usr/bin/git changes wd

  REQUIRE(!wex::build(wex::path_lexer("Makefile"))); // no set_handler_out

  REQUIRE(!wex::build(wex::path_lexer("xxx")));
  REQUIRE(!wex::build(wex::path_lexer("make.tst")));
  REQUIRE(!wex::build(wex::path_lexer("/usr/bin/git")));
#endif
}
