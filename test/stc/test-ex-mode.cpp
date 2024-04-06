////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-mode.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>

#include "test.h"

TEST_CASE("wex::ex-mode")
{
  auto* stc = get_stc();
  auto* ex  = &stc->get_vi();
  ex->use(wex::ex::mode_t::EX);
  stc->DocumentStart();

  SUBCASE("find")
  {
    wex::file ifs("test.md", std::ios_base::in);
    ex->ex_stream()->stream(ifs);
    REQUIRE(ex->command(":/w/"));
    REQUIRE(ex->ex_stream()->get_current_line() == 2);
    REQUIRE(ex->command("://"));
    REQUIRE(ex->ex_stream()->get_current_line() == 9);
    REQUIRE(ex->ex_stream()->get_previous_line());
    REQUIRE(ex->command(":??"));
    REQUIRE(ex->ex_stream()->get_current_line() == 2);
  }

  SUBCASE("print")
  {
    wex::file ifs("test.md", std::ios_base::in);
    ex->ex_stream()->stream(ifs);
    const std::string check("# Markdown\n\n- test for opening a Markdown "
                            "document (and used in test-ex-stream)\n");
    REQUIRE(ex->command(":p"));
    REQUIRE(ex->get_print_text() == "# Markdown\n");

    REQUIRE(ex->command(":p#"));
    REQUIRE(ex->get_print_text() == "     1 # Markdown\n");

    REQUIRE(ex->command(":l"));
    REQUIRE(ex->get_print_text() == "# Markdown$\n");

    REQUIRE(ex->command(":2z="));
    CAPTURE(ex->get_print_text());
    REQUIRE(ex->get_print_text().starts_with("---------"));
    REQUIRE(ex->get_print_text().ends_with("--------\n"));

    REQUIRE(ex->command(":1,3p"));
    REQUIRE(ex->get_print_text() == check);
  }

  ex->use(wex::ex::mode_t::VISUAL);
}
