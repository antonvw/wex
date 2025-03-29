////////////////////////////////////////////////////////////////////////////////
// Name:      test-printing.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/printing.h>
#include <wex/test/test.h>

TEST_CASE("wex::printing")
{
  REQUIRE(wex::printing::get() != nullptr);
  REQUIRE(wex::printing::get()->get_printer() != nullptr);
  REQUIRE(wex::printing::get()->get_html_printer() != nullptr);
  auto* old = wex::printing::get();
  REQUIRE(wex::printing::get()->set(nullptr) == old);
  REQUIRE(wex::printing::get(false) == nullptr);
  REQUIRE(wex::printing::get(true) != nullptr);

  SECTION("print_caption")
  {
    REQUIRE(wex::print_caption(wex::path("test")).contains("test"));
  }

  SECTION("print_footer")
  {
    REQUIRE(wex::print_footer().contains("@"));
  }

  SECTION("print_header")
  {
    REQUIRE(wex::print_header(wex::path_lexer(wex::test::get_path("test.h")))
              .contains("test"));
  }
}
